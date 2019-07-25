'use strict';

const assert = require('assert');

const { metacall, metacall_load_from_file } = require('../index.js');

describe('metacall', () => {
	describe('require', () => {
		it('functions metacall and metacall_load_from_file must be defined', () => {
			assert.notStrictEqual(metacall, undefined);
			assert.notStrictEqual(metacall_load_from_file, undefined);
		});
	});

	describe('load', () => {
		it('metacall_load_from_file (mock)', () => {
			assert.strictEqual(metacall_load_from_file('mock', ['asd.mock']), undefined); /* TODO: Do not return undefined */
		});
		it('metacall_load_from_file (py)', () => {
			assert.strictEqual(metacall_load_from_file('py', ['example.py']), undefined); /* TODO: Do not return undefined */
		});
		it('metacall_load_from_file (rb)', () => {
			assert.strictEqual(metacall_load_from_file('rb', ['second.rb']), undefined); /* TODO: Do not return undefined */
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
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7f1c3b04a025]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7f1c3b04a25c]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7f1c3af23904]
				54: /usr/lib/x86_64-linux-gnu/libruby-2.3.so.2.3 [0x7f1c3afd581e]
				54: /lib/x86_64-linux-gnu/libpthread.so.0 [0x7f1c4b92e0e0]
				54: /usr/bin/node [0x8d5ea0]
				54: /usr/bin/node [0x8d660c]
				54: /usr/bin/node(_ZN2v88internal25FunctionCallbackArguments4CallEPFvRKNS_20FunctionCallbackInfoINS_5ValueEEEE+0x193) [0xa94a43]
				54: /usr/bin/node [0xb0bbec]
				54: /usr/bin/node(_ZN2v88internal21Builtin_HandleApiCallEiPPNS0_6ObjectEPNS0_7IsolateE+0xaf) [0xb0c83f]
			*/
			/* assert.strictEqual(metacall('get_second', 5, 12), 12); */
		});
	});
});
