const vm = require('vm');
const r = require('repl');
const { command_register, command_parse, command_complete } = require('./parser')

// Method for generating a continuous chain of promises for allowing the repl
// evaluation to be resolved from outside (in the CLI main loop)
const new_repl_promise = () => {
	let promise_resolve = null;
	let promise_reject = null;
	const promise = new Promise(function(resolve, reject) {
		promise_resolve = resolve;
		promise_reject = reject;
	});

	const wait = async () => {
		// Await to the REPL promise to be resolved from the evaluator
		const p = await promise;

		// Reset the global promise from the REPL
		repl_promise = new_repl_promise();
		return p
	};

	return { resolve: promise_resolve, reject: promise_reject, wait };
}

let repl_promise = new_repl_promise();

// Show welcome message
console.log('Welcome to Tijuana, tequila, sexo & marijuana.');

// Start REPL
const repl = r.start({
	prompt: '\u03BB ',
	useGlobal: false,
	ignoreUndefined: true,
	preview: true,
	eval: evaluator,
});

function evaluator(cmd, context, file, cb) {
	// TODO: Implement parser for cmd
	repl_promise.resolve([command_parse(cmd.trim()), cb]);
}

// Complete function (hook it in order to allow inline autocompletion)
const _completer = repl.completer.bind(repl);
repl.completer = function(line, cb) {
	// Hook the completer callback in order to inject our own completion results
	const wrap = (err, result) => {
		// TODO: Generate autocompletion array
		cb(err, [['call'], line]);
	};
	_completer(line, wrap);
}

// Clear context and commands
repl.context = vm.createContext({});
repl.commands = {};

// On close event reject the repl promise
repl.on('close', () => {
	repl_promise.reject();
});

module.exports = {
	evaluate: async () => {
		return await repl_promise.wait();
	},
	command_register,
	close: () => {
		repl.close();
	}
}

setTimeout(() => repl.close(), 5000);
