const { metacall, metacall_load_from_file } = require('metacall');
const assert = require('node:assert').strict;

metacall_load_from_file('c', ['./bench.c']);

const result = metacall('bench_xyz789', 1234, 15);

console.log(result);

assert.ok(typeof result === 'number', `Expected float, but got ${typeof result}`);
assert.notStrictEqual(result, 0.0, `Expected non-zero, but got ${result}`);
