#!/usr/bin/env node
'use strict';

const path = require('path');
const os = require('os');
const fs = require('fs');
const assert = require('assert');

const {
	initialize,
	discover,
	clear,
	load_from_memory,
	load_from_file,
	destroy,
} = require('../build/bootstrap.js');

// Tests
initialize();

const inspect = (handle, equal = assert.notDeepStrictEqual) => {
	assert(handle !== null);
	const json = discover(handle);
	equal(json, {});
	console.log(json);
	clear(handle);
};

// Test load from memory
inspect(load_from_memory('memory_module', `
export function mem_sum(left: number, right: number): number {
	return left + right;
}
export async function mem_sum_async(left: number, right: number): number {
	return left + right;
}
`, {}));

// Test load from memory without exports
inspect(load_from_memory('memory_module_empty', `console.log('A')`, {}), assert.deepStrictEqual);

const runTestFromFile = (p, files) => {
	// We must set the current path in order to avoid bootstrap to load the tsconfig from lib
	process.chdir(p);
	inspect(load_from_file(files.map(f => path.join(p, f))));
};

// Testing from file with tsconfig
runTestFromFile(path.join(path.resolve(__dirname), 'script'), [ 'script.ts' ]);

// Testing polyfilling from ES2019 to ES2015
runTestFromFile(path.join(path.resolve(__dirname), 'from9to5'), [ 'from9to5.ts' ]);

// TODO: This function fails with [TypeError: Object.fromEntries is not a function]
// runTestFromFile(path.join(path.resolve(__dirname), 'lib'), [ 'lib.ts' ]);

// Testing default tsconfig
(() => {
	const tempDir = path.join(os.tmpdir(), 'ts-loader-bootstrap-test');
	if (!fs.existsSync(tempDir)) {
		fs.mkdirSync(tempDir);
	}
	const fileName = 'default.ts';
	fs.copyFileSync(path.join(path.resolve(__dirname), 'default', fileName), path.join(tempDir, fileName));
	runTestFromFile(tempDir, [ fileName ]);
})();

// Test a non existent file
(() => {
	const tempDir = path.join(os.tmpdir(), 'ts-loader-bootstrap-test-empty');
	if (!fs.existsSync(tempDir)) {
		fs.mkdirSync(tempDir);
	}
	assert(load_from_file([ path.join(tempDir, 'not-found.ts') ]) === null);
})();

// TODO: Test a file that compiles but fails to load

destroy();
