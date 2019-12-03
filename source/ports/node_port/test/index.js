'use strict';

const assert = require('assert');

const { metacall, metacall_load_from_file, metacall_inspect, metacall_logs } = require('../index.js');

describe('metacall', () => {
	describe('require', () => {
		it('functions metacall and metacall_load_from_file must be defined', () => {
			assert.notStrictEqual(metacall, undefined);
			assert.notStrictEqual(metacall_load_from_file, undefined);
			assert.notStrictEqual(metacall_inspect, undefined);
			assert.notStrictEqual(metacall_logs, undefined);
		});
	});

	describe('logs', () => {
		it('metacall_logs', () => {
			assert.strictEqual(metacall_logs(), undefined);
		});
	});

	describe('load', () => {
		it('metacall_load_from_file (mock)', () => {
			assert.strictEqual(require('asd.mock'), undefined); /* TODO: Do not return undefined */
		});
		it('metacall_load_from_file (py)', () => {
			assert.strictEqual(require('example.py'), undefined); /* TODO: Do not return undefined */
		});
		it('metacall_load_from_file (rb)', () => {
			assert.strictEqual(/*require('second.rb')*/ undefined, undefined); /* TODO: Do not return undefined */
		});
	});

	describe('inspect', () => {
		it('metacall_inspect', () => {
			const json = metacall_inspect();
			console.log(JSON.stringify(json));
			assert.notStrictEqual(json, undefined);
		});
	});

	describe('call', () => {
		it('metacall (mock)', () => {
			assert.strictEqual(metacall('my_empty_func'), 1234);
		});
		it('metacall (py)', () => {
			assert.strictEqual(metacall('multiply', 2, 2), 4);
		});
		it('metacall (rb)', () => {
			/* TODO: This creates a segmentation fault, it seems due to lack of thread-safety */
			/*
				54: -- C level backtrace information -------------------------------------------
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7fcfd8d27025]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7fcfd8d2725c]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7fcfd8c00904]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7fcfd8cb281e]
				54: /lib/x86_64-linux-gnu/libpthread.so.0 [0x7fcfdd60a0e0]
				54: [0x3089380fa9ff]
			*/
			/* assert.strictEqual(metacall('get_second', 5, 12), 12); */
		});
	});
});
