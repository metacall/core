const assert = require('assert').strict;
const { command_register, command_parse } = require('./cli_cmd_plugin')

command_register('help', 'METACALL_BOOL', 'h');

assert.deepEqual(command_parse([]), [{}, []]);
assert.deepEqual(command_parse(['-h']), [{ help: true }, []]);
assert.deepEqual(command_parse(['--help']), [{ help: true }, []]);
assert.deepEqual(command_parse(['--help', 'a.js']), [{ help: true }, [ 'a.js' ]]);
