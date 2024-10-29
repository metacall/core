const util = require('util');
const path = require('path');
const fs = require('fs');

const options = {};

function command_initialize(plugin_path) {
	/* Initialize all CMD descriptors
	*  This will load all plugin descriptors like:
	*  plugins/cli/cmd/${plugin_name}/${plugin_name}_cmd.js
	*
	*  The expected format is the following (e.g: cli_help_plugin.js):
	*  module.exports = {
	*      'help': {
	*          short: 'h',
	*          type: 'METACALL_BOOL' // This field is required
	*          fn: () => {} // Function will be executed when the command is called
	*          destroy: () => {} // Function will be executed at the end of the CLI lifecycle
	*      }
	*  };
	*/
	const cmd_path = path.join(plugin_path, 'cli', 'cmd');
	const files = (() => {
		try {
			return fs.readdirSync(cmd_path);
		} catch (e) {
			/* If the directory does not exist, return no files */
			if (e !== undefined && e.code === 'ENOENT') {
				return []
			}

			/* Otherwise, rethrow the exception */
			throw e;
		}
	})();

	for (const file of files) {
		const file_path = path.join(cmd_path, file);
		const file_stat = fs.statSync(file_path);

		if (file_stat.isDirectory()) {
			const descriptor_path = path.join(file_path, `${file}_cmd.js`);
			try {
				const descriptor_stat = fs.statSync(descriptor_path);

				if (descriptor_stat.isFile()) {
					const descriptor = require(descriptor_path);
					options = { ...options, ...descriptor };
				}
			} catch (e) {
				/* Skip */
			}
		}
	}
}

function command_register(cmd, { short, type, multiple, func, destroy }) {
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

	if (func) {
		options[cmd]['func'] = func;
	}

	if (destroy) {
		options[cmd]['destroy'] = destroy;
	}
}

function command_parse(...args) {
	const { values, positionals } = util.parseArgs({
		args,
		options,
		allowPositionals: true
	});

	return [ { ...values }, positionals ];
}

function command_function(cmd) {
	const COMMAND_NOT_REGISTERED = 0;
	const COMMAND_FUNCTION_UNDEFINED = 1;

	if (options[cmd] === undefined) {
		return COMMAND_NOT_REGISTERED;
	}

	if (options[cmd].func === undefined) {
		return COMMAND_FUNCTION_UNDEFINED;
	}

	return options[cmd].func;
}

function command_destroy() {
	for (const { destroy } of Object.values(options)) {
		if (destroy) {
			destroy();
		}
	}
}

module.exports = {
	command_initialize,
	command_register,
	command_parse,
	command_function,
	command_destroy
};
