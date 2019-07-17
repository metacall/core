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
		it('metacall_load_from_file must load correctly', () => {
			assert.strictEqual(metacall_load_from_file('mock', ['asd.mock']), undefined); /* TODO: Do not return undefined */
		});
	});

	describe('call', () => {
		it('metacall must call correctly', () => {
			assert.strictEqual(metacall('my_empty_func'), 1234);
		});
	});
});
