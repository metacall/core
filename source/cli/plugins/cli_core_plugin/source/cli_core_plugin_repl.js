const loaders = '^(mock|py|node|rb|cs|cob|ts|js|file|wasm|rs|c|rpc|ext|java)';
const all_except_whitespaces = '^[^ \r\n\t\f\v]+';
const func = '^[a-zA-Z0-9_\.]+\\(.*\\)$';
const anychar = '.+';

module.exports = {
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
	await: {
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
	},
	exit: {
		regexes: [],
		types: [],
	}
};
