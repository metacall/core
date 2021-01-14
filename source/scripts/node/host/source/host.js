#!/usr/bin/env node

'use strict';

const path = require('path');

/* Load MetaCall addon */
const addon = (() => {

	const LIBRARY_PATH = process.env.LOADER_LIBRARY_PATH;

	const folders = [
		path.join(__dirname, 'build'),
		__dirname,
		process.cwd(),
		LIBRARY_PATH,
		path.join(LIBRARY_PATH, 'build'),
		path.join(LIBRARY_PATH, 'node_modules', 'metacall'),
		path.join(LIBRARY_PATH, 'node_modules', 'metacall', 'build'),
		'/usr/local/lib',
	];

	const names = [
		'node_loaderd',
		'node_loader',
	];

	/* Load addon */
	return (() => {
		for (let folder of folders) {
			for (let name of names) {
				try {
					const location = path.join(folder, `${name}.node`);
					const port = require(location);

					if (port) {
						console.log(`NodeJS Port found at location: ${location}`);
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

const script = `#!/usr/bin/env python3

import os
import sys

sys.path.append(os.environ['PY_PORT_LIBRARY_PATH']);

from metacall import metacall

def b():
	return metacall('c');
`;

addon.metacall_load_from_memory('py', script);

function a() {
	console.log('-------------------------------------------');
	console.log(addon.metacall_inspect());
	console.log('-------------------------------------------');
	return addon.metacall('b');
}

function c() {
	return 3.0;
}

module.exports = {
	a,
	c,
};
