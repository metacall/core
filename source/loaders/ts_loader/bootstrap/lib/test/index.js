#!/usr/bin/env node
'use strict';

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

const inspect = (handle) => {
	const json = discover(handle);
	console.log(json);
	clear(handle);
};

inspect(load_from_memory('memory_module', `
export function mem_sum(left: number, rigth: number): number {
	return left + rigth;
}
export async function mem_sum_async(left: number, rigth: number): number {
	return left + rigth;
}
`, {}));

inspect(load_from_memory('memory_module_empty', `console.log('A')`, {}));

inspect(load_from_file(['./script.ts']));

destroy();
