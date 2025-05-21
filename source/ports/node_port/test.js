/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

const Mocha = require('mocha');
const fs = require('fs');
const path = require('path');

const mocha = new Mocha();
const testDir = path.resolve(__dirname, 'test');

fs.readdirSync(testDir).filter((file) => {
	return path.extname(file) === '.js';
}).forEach((file) => {
	mocha.addFile(
		path.join(testDir, file)
	);
});

// Set timeout to 5 min
mocha.timeout(300000);

const waitForMocha = async () => {
	// Promisfy mocha tests
	return new Promise((resolve, reject) => mocha.run(failures => failures ? reject(failures) : resolve()));
};

void (async () => {
	try {
		// Run the tests
		await waitForMocha();
	} catch (failures) {
		if (failures !== 0) {
			process.exit(1);
		}
	}

	console.log('Tests passed without errors');
})();
