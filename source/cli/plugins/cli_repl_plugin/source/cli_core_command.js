const loaders = '^(mock|py|node|rb|cs|cob|ts|js|file|wasm|rs|c|rpc|ext|java)';
const all_except_whitespaces = '^[^ \r\n\t\f\v]+';
const func = '^[a-zA-Z0-9_]+\\(.*\\)$';
const anychar = '.+';

/*
 * Normally this will be registered in the target plugin, check
 * the cli_repl_plugin.js exports to check out how to register commands
 * from the plugin initializer (for example, inside cli_core_plugin.cpp in
 * the cli_core_plugin function). But we consider this is a necesary plugin
 * of the CLI, which implements basic core functionalities for the CLI. In
 * that case we can assume this dependency and register from here the REPL
 * commands of an external plugin. 
 */
const cli_core_command_map = {
	load: {
		regexes: [loaders, all_except_whitespaces], /* Match everything except whitespaces, paths with whitespaces are not supported */
		types: ['METACALL_STRING', 'METACALL_ARRAY'],
	},
	inspect: {
		regexes: [],
		types: [],
	},
	eval: {
		regexes: [loaders, anychar], /* Match any char except by newline */
		types: ['METACALL_STRING', 'METACALL_STRING'],
	},
	call: {
		regexes: [func], /* Match any function call like: func_abc_3423("asd", 3434, 0.2) */
		types: ['METACALL_STRING'],
	},
	clear: {
		regexes: [loaders, all_except_whitespaces], /* Match everything except whitespaces, paths with whitespaces are not supported */
		types: ['METACALL_STRING', 'METACALL_STRING'],
	},
	copyright: {
		regexes: [],
		types: [],
	},
	help: {
		regexes: [],
		types: [],
	},
	debug: {
		regexes: [all_except_whitespaces],
		types: ['METACALL_ARRAY'],
	}
};

module.exports = { cli_core_command_map };
