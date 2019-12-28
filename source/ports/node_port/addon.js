'use strict';

const Path = require('path');

/* Load MetaCall addon */
module.exports = (() => {
	let installPath = '';

	try {
		installPath = require('./installPath');
	} catch (e) {
		if (e.code !== 'MODULE_NOT_FOUND') {
			throw e;
		}
	}

	const LIBRARY_PATH = process.env.LOADER_LIBRARY_PATH || installPath;

	const paths = [
		Path.join(__dirname, 'build'),
		__dirname,
		process.cwd(),
		LIBRARY_PATH,
		Path.join(LIBRARY_PATH, 'build'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall'),
		Path.join(LIBRARY_PATH, 'node_modules', 'metacall', 'build'),
		'/usr/local/lib',
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
