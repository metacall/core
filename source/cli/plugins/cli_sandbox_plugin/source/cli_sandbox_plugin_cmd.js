const { command_register } = require('../../cli_cmd_plugin/source/cli_cmd_plugin');

function cli_sandbox_plugin_cmd_initialize() {
  command_register('sandbox', 'METACALL_BOOL', 's');              // s -> sandbox
  command_register('allow-io', 'METACALL_BOOL', 'sio');           // sio -> sandbox-io
  command_register('allow-sockets', 'METACALL_BOOL', 'sn');       // sn -> sandbox-network
  command_register('allow-ipc', 'METACALL_BOOL', 'sipc');         // sipc -> sandbox-ipc
  command_register('allow-process', 'METACALL_BOOL', 'sp');       // sp -> sandbox-process
  command_register('allow-filesystems', 'METACALL_BOOL', 'sf');   // sf -> sandbox-filesystem
  command_register('allow-time', 'METACALL_BOOL', 'st');          // st -> sandbox-time
  command_register('allow-memory', 'METACALL_BOOL', 'sm');        // sm -> sandbox-memory
  command_register('allow-signals', 'METACALL_BOOL', 'ss');       // ss -> sandbox-signals
}

module.exports = {
  cli_sandbox_plugin_cmd_initialize
};
