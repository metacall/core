#!/usr/bin/env python3

import os
import sys

sys.path.append(os.environ['PORT_LIBRARY_PATH']);

try:
	from _py_port import metacall, metacall_load_from_memory
except ImportError:
	from _py_portd import metacall, metacall_load_from_memory

script = '''#!/usr/bin/env node

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
		'libnode_portd',
		'libnode_port',
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

function b() {
	return addon.metacall('c');
}

module.exports = {
	b
};
'''

metacall_load_from_memory('node', script);

def a():
	result = metacall('b')
	print('Result call from b:')
	print(result)
	return result;

def c():
	return 3.0;
