'use strict';

let addon = null;

try {
	addon = require('./build/node_port.node');
} catch (e) {
	if (e.code !== 'MODULE_NOT_FOUND') {
		throw e;
	}

	addon = require('./build/node_portd.node');
}

/* TODO: Override require and monkey patch the functions */

module.exports = {
	metacall: (name, ...args) => {
		if (Object.prototype.toString.call(name) !== '[object String]') {
			throw Error('Function name should be of string type.');
		}

		/* TODO: This is not working */
		addon.metacall(name, ...args);
	},

	metacall_load_from_file: (tag, paths) => {
		if (Object.prototype.toString.call(tag) !== '[object String]') {
			throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
		}

		if (!(paths instanceof Array)) {
			throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
		}

		addon.metacall_load_from_file(tag, paths);
	},
};
