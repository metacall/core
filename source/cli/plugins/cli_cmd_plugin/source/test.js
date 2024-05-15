const assert = require('assert').strict;
const { command_register, command_parse } = require('./cli_cmd_plugin')

command_register('empty', {});
command_register('help', { short: 'h', type: 'METACALL_BOOL' });
command_register('option', { short: 'o', type: 'METACALL_STRING' });
command_register('multiple', { short: 'm', type: 'METACALL_STRING', multiple: true });

assert.deepEqual(command_parse(), [{}, []]);
assert.deepEqual(command_parse('--empty'), [{ empty: true }, []]);
assert.deepEqual(command_parse('-h'), [{ help: true }, []]);
assert.deepEqual(command_parse('--help'), [{ help: true }, []]);
assert.deepEqual(command_parse('--help', 'a.js'), [{ help: true }, ['a.js']]);
assert.deepEqual(command_parse('--option=hello', 'a.js'), [{ option: 'hello' }, ['a.js']]);
assert.deepEqual(command_parse('--multiple=a', '--multiple=b', '--multiple=c'), [{ multiple: ['a', 'b', 'c'] }, []]);
