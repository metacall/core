const util = require('util');

const options = {};

function command_register(cmd, type, short, multiple) {
	const type_map = {
		METACALL_STRING: 'string',
		METACALL_BOOL: 'boolean'
	};

	options[cmd] = {
		type: type_map[type || 'METACALL_BOOL']
	};

	if (short) {
		options[cmd]['short'] = short;
		multiple
	}

	if (multiple) {
		options[cmd]['multiple'] = multiple;
	}
}

function command_parse(args) {
	const { values, positionals } = util.parseArgs({
		args,
		options,
		allowPositionals: true
	});

	return [ { ...values }, positionals ];
}

module.exports = {
	command_register,
	command_parse
};
