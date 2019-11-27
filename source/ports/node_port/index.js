'use strict';

const Path = require('path');
const Module = require('module');

/* Load MetaCall addon */
const addon = (() => {
	const LIBRARY_PATH = process.env.LOADER_LIBRARY_PATH || '';

	const paths = [
		Path.join(__dirname, 'build'),
		__dirname,
		process.cwd(),
		LIBRARY_PATH,
		Path.join(LIBRARY_PATH, 'build'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall', 'build'),
	];

	const names = [
		'libnode_portd',
		'libnode_port',
	];

	/* Set NODE_PATH for finding metacall lib */
	/*
	process.env.NODE_PATH = `${process.env.NODE_PATH}:${paths.join(':')}`;
	Module._initPaths();
	*/

	/* Load addon */
	return (() => {
		for (let path of paths) {
			for (let name of names) {
				try {
					const port = require(Path.join(path, `${name}.node`));

					if (port) {
						return port;
					}
				} catch (e) {
					if (e.code !== 'MODULE_NOT_FOUND') {
						throw e;
					}
				}
			}
		}
	})();
})();

const node_require = Module.require;

const metacall_require = (tag, name) => {
	// TODO: Inspect the current handle and append it to an object mocking the function calls with metacall
	return addon.metacall_load_from_file(tag, [ name ]);
};

/* Monkey patch require for simplifying load */
Module.prototype.require = (id) => {

	const tags = {
		mock: 'mock',
		py: 'py',
		rb: 'rb',
		cs: 'cs',
		/*dll: 'cs',*/
	};

	const index = id.lastIndexOf('.');

	if (index === -1) {
		return node_require(id);
	} else {
		// Load the module
		const extension = id.substr(index + 1);
		const tag = tags[extension];

		if (tag) {
			return metacall_require(tag, id);
		} else {
			return node_require(id);
		}
	}
};

/* Export the API */
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
