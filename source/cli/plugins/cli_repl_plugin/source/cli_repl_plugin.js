const vm = require('vm');
const r = require('repl');
const fs = require('fs');
const path = require('path');
const { repl_register, repl_register_from_file, repl_parse, repl_completer } = require('./parser');

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

let repl_promise = null;
let repl = null;

function repl_initialize(plugin_path) {
	repl_promise = [new_repl_promise()];

	/* Initialize all REPL descriptors
	*  This will load all plugin descriptors like:
	*  plugins/cli/repl/${plugin_name}/${plugin_name}_repl.js
	*/
	const repl_path = path.join(plugin_path, 'cli', 'repl');
	const files = (() => {
		try {
			return fs.readdirSync(repl_path);
		} catch (e) {
			/* If the directory does not exist, return no files */
			if (e?.code === 'ENOENT') {
				return []
			}

			/* Otherwise, rethrow the exception */
			throw e;
		}
	})();

	for (const file of files) {
		const file_path = path.join(repl_path, file);
		const file_stat = fs.statSync(file_path);

		if (file_stat.isDirectory()) {
			const descriptor_path = path.join(file_path, `${file}_repl.js`);
			try {
				const descriptor_stat = fs.statSync(descriptor_path);

				if (descriptor_stat.isFile()) {
					repl_register_from_file(descriptor_path);
				}
			} catch (e) {
				/* Skip */
			}
		}
	}

	/* Show welcome message */
	console.log('Welcome to Tijuana, tequila, sexo & marijuana.');

	/* Start REPL */
	repl = r.start({
		prompt: '\u03BB ',
		useGlobal: false,
		ignoreUndefined: true,
		preview: true,
		eval: evaluator,
		completer: completer
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
			const result = repl_parse(cmd.trim());
			promise.resolve([result, cb]);
		} catch (e) {
			promise.resolve([e, cb]);
		}

		repl_promise.push(new_repl_promise());
	}

	function completer(line) {
		const completions = repl_completer();
		const hits = completions.filter(c => c.startsWith(line));
		return [hits.length ? hits : completions, line];
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
}

/* Usage:
 * evaluate().then(data => {
 * 	console.log(data);
 * 	data[1](null, 'result of data execution');
 * }).catch(e => {
 * 	console.error(e);
 * });
*/
const repl_evaluate = async () => {
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
	repl_initialize,
	repl_evaluate,
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
	 * void *ret = metacallhv_s(repl_handle, "repl_register", args, sizeof(args) / sizeof(args[0]));
	 *
	 * metacall_value_destroy(args[0]);
	 * metacall_value_destroy(args[1]);
	 * metacall_value_destroy(args[2]);
	 *
	 * metacall_value_destroy(ret);
	 */
	repl_register,

	/* This function is exported so it can be called from other plugins:
	 *
	 * void *repl_handle = metacall_handle("ext", "cli_repl_plugin");
	 * void *args[] = {
	 * 	metacall_value_create_string("./cli_core_plugin_repl.js", sizeof("./cli_core_plugin_repl.js") - 1),
	 * };
	 *
	 * void *ret = metacallhv_s(repl_handle, "repl_register_from_file", args, sizeof(args) / sizeof(args[0]));
	 *
	 * metacall_value_destroy(args[0]);
	 * metacall_value_destroy(ret);
	 */
	repl_register_from_file,

	repl_close: () => {
		if (repl) {
			repl.close();
			repl = null;
		}
		repl_promise = null;
	}
};
