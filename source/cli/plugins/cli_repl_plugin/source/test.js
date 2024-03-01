const assert = require('assert').strict;
const { repl_register_map, repl_parse } = require('./parser');

/* Integration Test */
const cli_core_repl_map = require('../../cli_core_plugin/source/cli_core_plugin_repl');

/* Register commands */
repl_register_map(cli_core_repl_map);

assert.deepEqual(repl_parse('load py a.py b.py c.py ./a/b/c/d.py'),
	['load', 'py', ['a.py', 'b.py', 'c.py', './a/b/c/d.py']]);

assert.deepEqual(repl_parse('inspect'),
	['inspect']);

assert.deepEqual(repl_parse('eval node console.log("hello world")'),
	['eval', 'node', 'console.log("hello world")']);

assert.deepEqual(repl_parse('call asdf(234, 34, 2, 3, 4, 5)'),
	['call', 'asdf(234, 34, 2, 3, 4, 5)']);

assert.deepEqual(repl_parse('call asdf()'),
	['call', 'asdf()']);

assert.throws(() => {
	repl_parse('call asdf(234, 34, 2, 3, 4, 5',
		"Failed to parse the command: 'asdf(234, 34, 2, 3, 4, 5' of type METACALL_STRING, the expected regex was: /^[a-zA-Z0-9_]+(.+)$/g");
})

assert.throws(() => {
	repl_parse('call asdf234, 34, 2, 3, 4, 5)',
		"Failed to parse the command: 'asdf234, 34, 2, 3, 4, 5)' of type METACALL_STRING, the expected regex was: /^[a-zA-Z0-9_]+(.+)$/g");
})

assert.deepEqual(repl_parse('clear py a.py'),
	['clear', 'py', 'a.py']);

assert.deepEqual(repl_parse('copyright'),
	['copyright']);

assert.deepEqual(repl_parse('help'),
	['help']);

assert.deepEqual(repl_parse('debug a b c 1 2 3'),
	['debug', ['a', 'b', 'c', '1', '2', '3']]);

/* Unit Test */
const letters = '^[a-zA-Z]+';
const numbers = '^[0-9]+';

/* Register test commands */
repl_register_map({
	/* Following commands are for testing purposes only */
	string_array_string: {
		regexes: [letters, numbers, letters],
		types: ['METACALL_STRING', 'METACALL_ARRAY', 'METACALL_STRING']
	}
});

assert.deepEqual(repl_parse('string_array_string yeet 1 2 3 4 5555 6 7 8 9 yeet'),
	['string_array_string', 'yeet', ['1', '2', '3', '4', '5555', '6', '7', '8', '9'], 'yeet']);
