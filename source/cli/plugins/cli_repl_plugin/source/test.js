const assert = require('assert').strict;
const { command_register, command_parse } = require('./parser');

const loaders = "^(mock|py|node|rb|cs|cob|ts|js|file|wasm|rs|c|rpc|ext|java)";
const path = "((?:[^\/]*\/)*)(.*)";
const func = "^[a-zA-Z0-9_]+\(.+\)$";
const anychar = ".+";
const parser_map = {
	load: {
		regexes: [loaders, path],
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
		types: ['METACALL_STRING', 'METACALL_STRING'],
	},
	clear: {
		regexes: [loaders, anychar], /* Match any char except by newline */
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
		regexes: [anychar],
		types: ['METACALL_ARRAY'],
	},
};

// Register commands
Object.keys(parser_map).forEach(key => command_register(key, parser_map[key].regexes, parser_map[key].types));

console.log(command_parse('call asdf(234, 34, 2, 3, 4, 5)'));
