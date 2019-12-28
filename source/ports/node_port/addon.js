/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
