const vm = require('vm');
const r = require('repl');
const { command_register, command_register_map, command_parse, command_complete } = require('./parser');
const { cli_core_command_map } = require('./cli_core_command');

/* Register CLI Core command map into the parser */
command_register_map(cli_core_command_map);

/*
 * Method for generating a continuous chain of promises for allowing the repl
 * evaluation to be resolved from outside (in the CLI main loop)
 */
const new_repl_promise = () => {
	let promise_resolve = null;
	let promise_reject = null;
	const promise = new Promise(function(resolve, reject) {
		promise_resolve = resolve;
		promise_reject = reject;
	});

	const wait = async () => {
		/* Await to the REPL promise to be resolved from the evaluator */
		const p = await promise;
		return p;
	};

	return { resolve: promise_resolve, reject: promise_reject, wait };
}

let repl_promise = [new_repl_promise()];

/* Show welcome message */
console.log('Welcome to Tijuana, tequila, sexo & marijuana.');

/* Start REPL */
const repl = r.start({
	prompt: '\u03BB ',
	useGlobal: false,
	ignoreUndefined: true,
	preview: true,
	eval: evaluator
});

function evaluator(cmd, context, file, cb) {
	if (repl_promise === null) {
		cb(new Error('Invalid REPL Promise'), null);
	}

	if (repl_promise.length === 0) {
		repl_promise.push(new_repl_promise());
	}

	const promise = repl_promise[repl_promise.length - 1];

	try {
		const result = command_parse(cmd.trim());
		promise.resolve([result, cb]);
	} catch (e) {
		promise.resolve([e, cb]);
	}

	repl_promise.push(new_repl_promise());
}

/* Complete function (hook it in order to allow inline autocompletion) */
const _completer = repl.completer.bind(repl);
repl.completer = function(line, cb) {
	/* Hook the completer callback in order to inject our own completion results */
	const wrap = (err, result) => {
		/* TODO: Generate autocompletion array (command_complete) */
		cb(err, [['call'], line]);
	};
	_completer(line, wrap);
}

/* Clear context and commands */
repl.context = vm.createContext({});
repl.commands = {};

/* On close event reject the repl promise */
repl.on('close', () => {
	if (repl_promise !== null) {
		if (repl_promise.length === 1) {
			repl_promise[0].reject();
		}

		repl_promise.pop();
	}
});

/* Usage:
 * evaluate().then(data => {
 * 	console.log(data);
 * 	data[1](null, 'result of data execution');
 * }).catch(e => {
 * 	console.error(e);
 * });
*/
const evaluate = async () => {
	if (repl_promise !== null) {
		if (repl_promise.length === 0) {
			repl_promise.push(new_repl_promise());
		}

		const result = await repl_promise[0].wait();

		repl_promise.shift();

		return result;
	}

	return [ new Error('Invalid REPL Promise'), () => {} ];
};

module.exports = {
	evaluate,
	/* This function is exported so it can be called from other plugins:
	 *
	 * void *repl_handle = metacall_handle("ext", "cli_repl_plugin");
	 * void *args[] = {
	 * 	metacall_value_create_string("example", sizeof("example") - 1),
	 * 	metacall_value_create_array(NULL, 1),
	 * 	metacall_value_create_array(NULL, 1)
	 * };
	 *
	 * void *regexes = metacall_value_to_array(args[1]);
	 * regexes[0] = metacall_value_create_string("[a-zA-Z0-9_]", sizeof("[a-zA-Z0-9_]") - 1);
	 *
	 * void *types = metacall_value_to_array(args[2]);
	 * types[0] = metacall_value_create_string("METACALL_STRING", sizeof("METACALL_STRING") - 1);
	 *
	 * void *ret = metacallhv_s(repl_handle, "command_register", args, sizeof(args) / sizeof(args[0]));
	 *
	 * metacall_value_destroy(args[0]);
	 * metacall_value_destroy(args[1]);
	 * metacall_value_destroy(args[2]);
	 *
	 * metacall_value_destroy(ret);
	 */
	command_register,

	close: () => {
		repl_promise = null;
		repl.close();
	}
};
