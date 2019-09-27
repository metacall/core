'use strict';

const Path = require('path');
const Module = require('module');

/* Load MetaCall addon */
const addon = (() => {
	const LIBRARY_PATH = process.env.LOADER_LIBRARY_PATH || '';

	const paths = [
		__dirname,
		Path.join(__dirname, 'build'),
		process.cwd(),
		LIBRARY_PATH,
		Path.join(LIBRARY_PATH, 'build'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall', 'build'),
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

/* Monkey patch require for simplifying load */
Module.prototype.require = new Proxy(Module.prototype.require, {
	apply(target, module, args) {

		const node_require = () => {
			return Reflect.apply(target, module, args);
		};

		const metacall_require = (tag, name) => {
			addon.metacall_load_from_file(tag, [ name ]);
			// TODO: Inspect the current handle and append it to an object mocking the function calls with metacall
			return 1;
		};

		const loaders = {
			py: metacall_require,
			rb: metacall_require,
			cs: metacall_require,
			js: node_require,
			node: node_require,
		};

		const loadersTag = {
			py: 'py',
			rb: 'rb',
			cs: 'cs',
			/*dll: 'cs',*/
		};

		const extensions = Object.keys(loaders);

		const name = args[0];
		const index = name.lastIndexOf('.');

		if (index === -1) {
			return node_require();
		} else {
			// Load the module
			const ext = name.substr(index + 1);
			const loader = loaders[ext];

			if (loader) {
				return loader(loadersTag[ext], name);
			} else {
				throw new Error(`Extension ${ext} is not suported by MetaCall.`);
			}
		}
	}
});

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
