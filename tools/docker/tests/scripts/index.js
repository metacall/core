const { metacall, metacall_load_from_file } = require('metacall');
const assert = require('node:assert').strict;
const { promisify } = require('node:util');
const { execFile } = require('node:child_process');
const execFilePromises = promisify(execFile);

// C
(() => {
	metacall_load_from_file('c', ['./bench.c']);

	const result = metacall('bench_xyz789', 1234, 15);

	console.log(result);

	assert.ok(typeof result === 'number', `Expected float, but got ${typeof result}`);
	assert.notStrictEqual(result, 0.0, `Expected non-zero, but got ${result}`);
})();

// Rust
(async () => {
	// TODO: Remove this
	const { stdout } = await execFilePromises('getconf', ['LONG_BIT']);
	const bits = stdout.trim();

	if (process.arch !== 'x64' || bits !== '64') {
		console.log(`TODO: Rust not working for ${process.arch} architecture with ${bits} bits.`);
		return;
	}

	metacall_load_from_file('rs', ['./bench.rs']);

	const result = metacall('pairwise_suffix_sum', [4.4, 5.5, 6.6, 7.7]);

	console.log(result);

	assert.ok(typeof result === 'number', `Expected float, but got ${typeof result}`);
	assert.strictEqual(result, 369.04999999999995);
})();

// C#

// Commented out: cs_loader crashes on init in metacall/core:cli (exit 134).
// Re-enable once the loader install-path bug is fixed.

// (() => {
// 	metacall_load_from_file('cs', ['./sum.cs']);

// 	const result = metacall('sum_cs', 3, 4);
// 	console.log(result);
// 	assert.strictEqual(result, 7);

// 	const greeting = metacall('greet_cs', 'MetaCall');
// 	console.log(greeting);
// 	assert.strictEqual(greeting, 'Hello MetaCall from C#!');
// })();
