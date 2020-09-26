/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

'use strict';

const path = require('path');

/* Load MetaCall addon */
module.exports = (() => {
	let installPath = '';

	try {
		installPath = require(path.resolve(__dirname, './installPath.js'));
	} catch (e) {
		if (e.code !== 'MODULE_NOT_FOUND') {
			throw e;
		}
	}

	const LIBRARY_PATH = process.env.LOADER_LIBRARY_PATH || installPath;

	const folders = [
		LIBRARY_PATH,
		path.join(LIBRARY_PATH, 'build'),
		path.join(LIBRARY_PATH, 'node_modules', 'metacall'),
		path.join(LIBRARY_PATH, 'node_modules', 'metacall', 'build'),
		path.join(__dirname, 'build'),
		__dirname,
		process.cwd(),
		'/usr/local/lib',
	];

	const names = [
		'node_loaderd',
		'node_loader',
		'libnode_loaderd',
		'libnode_loader',
	];

	/* Set NODE_PATH for finding metacall lib */
	/*
	process.env.NODE_PATH = `${process.env.NODE_PATH}:${folders.join(':')}`;
	Module._initPaths();
	*/

	/* In order to support the loader and port implemented in the same plugin,
	the plugin will be loaded by metacall (dlopen) and by node at the same time (napi).
	NodeJS cannot load extensions with .dll or .so extensions, they must be .node.
	For solving this, we provide a temporal mock for allowing loading the system library
	like an extension, even if it has a different one.
	*/
	const extension = process.platform === 'win32' ? '.dll' : '.so';

	require.extensions[extension] = require.extensions['.node'];

	/* Load addon */
	const addon = (() => {
		for (let folder of folders) {
			for (let name of names) {
				try {
					const libPath = path.join(folder, `${name}${extension}`);
					const port = require(libPath);

					if (port) {
						console.log('MetaCall NodeJS Port Addon found at:', libPath);
						return port;
					}
				} catch (e) {
					if (e.code !== 'MODULE_NOT_FOUND') {
						delete require.extensions[extension];
						throw e;
					}
				}
			}
		}
	})();

	delete require.extensions[extension];

	if (addon === undefined) {
		console.log('Error when loading the MetaCall NodeJS Port Addon. NodeJS module not found.');
	}

	return addon;
})();
