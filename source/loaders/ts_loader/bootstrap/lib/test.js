#!/usr/bin/env node
'use strict';

const {
	initialize,
	discover,
	clear,
	load_from_memory,
	load_from_file,
	destroy,
} = require('./bootstrap.ts');

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
`, {}));

inspect(load_from_file(['./script.ts']));

destroy();
