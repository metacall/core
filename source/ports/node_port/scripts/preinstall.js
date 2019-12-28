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

if (require('./addon') === undefined) {
	const path = require('path');
	const fs = require('fs');

	const basePath = '/gnu/store';

	function findFolder(base, re) {
		const folders = [];

		if (!fs.existsSync(base)) {
			return folders;
		}

		const files = fs.readdirSync(base);

		for (let index = 0; index < files.length; ++index) {
			const filename = path.join(base, files[index]);
			const stat = fs.lstatSync(filename);
			if (stat.isDirectory()) {
				if (re.test(filename)) {
					folders.push(filename);
				}
			}
		}

		return folders;
	}

	const folders = findFolder(basePath, /.*-metacall-.*[^R]$/);

	if (folders.length == 0) {
		console.log('\x1b[31m\x1b[1m%s\x1b[0m', 'You do not have MetaCall installed or we cannot find it');
		console.log('\x1b[33m\x1b[1m%s\x1b[0m', 'If you do not have it installed, you have three options:');
		console.log('\x1b[1m%s\x1b[0m', '	1) Go to https://github.com/metacall/install and install it.');
		console.log('\x1b[1m%s\x1b[0m', '	2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.');
		console.log('\x1b[1m%s\x1b[0m', '	3) Be a x10 programmer and compile it by yourself, then define the install folder (if it is different from the default /usr/local/lib) in process.env.LOADER_LIBRARY_PATH.');
		console.log('\x1b[33m\x1b[1m%s\x1b[0m', 'If you have it installed in an non-standard folder, please define process.env.LOADER_LIBRARY_PATH.');

		process.exit(1);
	}

	const script = `'use strict';
module.exports = '${folders[0]}/lib';
`;

	fs.writeFileSync('./installPath.js', script);
}
