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

	/* Set NODE_PATH for finding metacall lib */
	/*
	process.env.NODE_PATH = `${process.env.NODE_PATH}:${folders.join(':')}`;
	Module._initPaths();
	*/

	/* Load addon */
	return (() => {
		for (let folder of folders) {
			for (let name of names) {
				try {
					const port = require(path.join(folder, `${name}.node`));

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
