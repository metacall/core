'use strict';

const Path = require('path');

const addon = (() => {
	const paths = [
		Path.join(__dirname, 'build'),
		process.cwd(),
		process.env.LOADER_LIBRARY_PATH,
	];

	const names = [
		'node_port',
		'node_portd',
	];

	const addon = (() => {
		for (let path of paths) {
			for (let name of names) {
				try {
					const addon = require(Path.join(path, `${name}.node`));

					if (addon) {
						return addon;
					}
				} catch (e) {
					if (e.code !== 'MODULE_NOT_FOUND') {
						throw e;
					}
				}
			}
		}
	})();

	return addon;
})();

/* TODO: Override require and monkey patch the functions */

module.exports = {
	metacall: (name, ...args) => {
		if (Object.prototype.toString.call(name) !== '[object String]') {
			throw Error('Function name should be of string type.');
		}

		return addon.metacall(name, ...args);
	},

	metacall_load_from_file: (tag, paths) => {
		if (Object.prototype.toString.call(tag) !== '[object String]') {
			throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
		}

		if (!(paths instanceof Array)) {
			throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
		}

		return addon.metacall_load_from_file(tag, paths);
	},
};
