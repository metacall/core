const assert = require('assert').strict;
const { command_register, command_parse } = require('./cli_cmd_plugin')

command_register('help', 'METACALL_BOOL', 'h');
command_register('option', 'METACALL_STRING', 'o');
command_register('multiple', 'METACALL_STRING', 'm', true);

assert.deepEqual(command_parse([]), [{}, []]);
assert.deepEqual(command_parse(['-h']), [{ help: true }, []]);
assert.deepEqual(command_parse(['--help']), [{ help: true }, []]);
assert.deepEqual(command_parse(['--help', 'a.js']), [{ help: true }, ['a.js']]);
assert.deepEqual(command_parse(['--option=hello', 'a.js']), [{ option: 'hello' }, ['a.js']]);
assert.deepEqual(command_parse(['--multiple=a', '--multiple=b', '--multiple=c']), [{ multiple: ['a', 'b', 'c'] }, []]);
