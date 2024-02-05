const assert = require('assert').strict;
const { command_register_map, command_parse } = require('./parser');
const { cli_core_command_map } = require('./cli_core_command');

/* Register commands */
command_register_map(cli_core_command_map);

assert.deepEqual(command_parse('load py a.py b.py c.py ./a/b/c/d.py'), {
	command: 'load',
	parameters: ['py', ['a.py', 'b.py', 'c.py', './a/b/c/d.py']]
});

assert.deepEqual(command_parse('inspect'), {
	command: 'inspect',
	parameters: []
});

assert.deepEqual(command_parse('eval node console.log("hello world")'), {
	command: 'eval',
	parameters: ['node', 'console.log("hello world")']
});

assert.deepEqual(command_parse('call asdf(234, 34, 2, 3, 4, 5)'), {
	command: 'call',
	parameters: ['asdf(234, 34, 2, 3, 4, 5)']
});

assert.deepEqual(command_parse('call asdf()'), {
	command: 'call',
	parameters: ['asdf()']
});

assert.throws(() => {
	command_parse('call asdf(234, 34, 2, 3, 4, 5',
		"Failed to parse the command: 'asdf(234, 34, 2, 3, 4, 5' of type METACALL_STRING, the expected regex was: /^[a-zA-Z0-9_]+(.+)$/g");
})

assert.throws(() => {
	command_parse('call asdf234, 34, 2, 3, 4, 5)',
		"Failed to parse the command: 'asdf234, 34, 2, 3, 4, 5)' of type METACALL_STRING, the expected regex was: /^[a-zA-Z0-9_]+(.+)$/g");
})

assert.deepEqual(command_parse('clear py a.py'), {
	command: 'clear',
	parameters: ['py', 'a.py']
});

assert.deepEqual(command_parse('copyright'), {
	command: 'copyright',
	parameters: []
});

assert.deepEqual(command_parse('help'), {
	command: 'help',
	parameters: []
});

assert.deepEqual(command_parse('debug a b c 1 2 3'), {
	command: 'debug',
	parameters: [['a', 'b', 'c', '1', '2', '3']]
});

/* Register test commands */
const letters = '^[a-zA-Z]+';
const numbers = '^[0-9]+';

command_register_map({
	/* Following commands are for testing purposes only */
	string_array_string: {
		regexes: [letters, numbers, letters],
		types: ['METACALL_STRING', 'METACALL_ARRAY', 'METACALL_STRING']
	}
});

assert.deepEqual(command_parse('string_array_string yeet 1 2 3 4 5555 6 7 8 9 yeet'), {
	command: 'string_array_string',
	parameters: ['yeet', ['1', '2', '3', '4', '5555', '6', '7', '8', '9'], 'yeet']
});
