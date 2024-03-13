const util = require('util');

const options = {};

function command_initialize(plugin_path) {
	/* Initialize all CMD descriptors
	*  This will load all plugin descriptors like:
	*  plugins/cli/cmd/${plugin_name}/${plugin_name}_cmd.js
	*/
	const cmd_path = path.join(plugin_path, 'cli', 'cmd', 'sandbox');
	const files = fs.readdirSync(cmd_path);

	for (const file of files) {
		const file_path = path.join(cmd_path, file);
		const file_stat = fs.statSync(file_path);

		if (file_stat.isDirectory()) {
			const descriptor_path = path.join(file_path, `${file}_cmd.js`);
			const descriptor_stat = fs.statSync(descriptor_path);

			if (descriptor_stat.isFile()) {
				const descriptor = require(descriptor_path);
				options = { ...options, ...descriptor };
			}
		}
	}
}

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
	command_initialize,
	command_register,
	command_parse
};
