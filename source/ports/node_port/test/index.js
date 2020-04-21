/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

const assert = require('assert');

const { metacall, metacall_load_from_file, metacall_handle, metacall_inspect, metacall_logs } = require('../index.js');

describe('metacall', () => {
	describe('require', () => {
		it('functions metacall and metacall_load_from_file must be defined', () => {
			assert.notStrictEqual(metacall, undefined);
			assert.notStrictEqual(metacall_load_from_file, undefined);
			assert.notStrictEqual(metacall_inspect, undefined);
			assert.notStrictEqual(metacall_logs, undefined);
		});
	});

	describe('load', () => {
		it('metacall_load_from_file (py)', () => {
			assert.strictEqual(metacall_load_from_file('py', [ 'helloworld.py' ] ), undefined);

			const script = metacall_handle('py', 'helloworld');
			assert.notStrictEqual(script, undefined);
			assert.strictEqual(script.name, 'helloworld');
		});
		it('metacall_load_from_file (rb)', () => {
			assert.strictEqual(metacall_load_from_file('rb', [ 'ducktype.rb' ]), undefined);

			const script = metacall_handle('rb', 'ducktype');
			assert.notStrictEqual(script, undefined);
			assert.strictEqual(script.name, 'ducktype');
		});
		it('require (mock)', () => {
			const asd = require('asd.mock');
			assert.notStrictEqual(asd, undefined);
			assert.strictEqual(asd.my_empty_func(), 1234);
			assert.strictEqual(asd.my_empty_func_str(), 'Hello World');
			assert.strictEqual(asd.my_empty_func_int(), 1234);
			assert.strictEqual(asd.new_args('a'), 'Hello World');
			assert.strictEqual(asd.two_str('1', '2'), 'Hello World');
			assert.strictEqual(asd.two_doubles(4.4, 5.5), 3.1416);
			assert.strictEqual(asd.three_str('a', 'b', 'c'), 'Hello World');
			assert.strictEqual(asd.mixed_args('a', 3, 4, 3.4, 'NOT IMPLEMENTED'), 65);
		});
		it('metacall_load_from_file (cob)', () => {
			assert.strictEqual(metacall_load_from_file('cob', [ 'say.cob' ]), undefined);

			const script = metacall_handle('cob', 'say');

			// Cobol tests are optional (in order to pass CI/CD)
			if (script) {
				assert.notStrictEqual(script, undefined);
				assert.strictEqual(script.name, 'say');

				assert.strictEqual(metacall('say', 'Hello, ', 'world!'), 0);
			}
		});
		it('require (py)', () => {
			const example = require('example.py');
			assert.notStrictEqual(example, undefined);
			assert.strictEqual(example.multiply(2, 2), 4);
			assert.strictEqual(example.divide(4.0, 2.0), 2.0);
			assert.strictEqual(example.sum(2, 2), 4);
			assert.strictEqual(example.strcat('2', '2'), '22');
			assert.deepEqual(example.return_array(), [1, 2, 3]);
			assert.deepEqual(example.return_same_array([1, 2, 3]), [1, 2, 3]);
		});
		it('require (rb)', () => {
			const second = require('second.rb');
			assert.notStrictEqual(second, undefined);
			assert.strictEqual(second.get_second(3, 4), 4);
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
			assert.strictEqual(metacall('three_str', 'a', 'b', 'c'), 'Hello World');
		});
		it('metacall (py)', () => {
			assert.strictEqual(metacall('multiply', 2, 2), 4);
			assert.deepEqual(metacall('return_array'), [1, 2, 3]);
			assert.deepEqual(metacall('return_same_array', [4, 5, 6]), [4, 5, 6]);
		});
		it('metacall (rb)', () => {
			assert.strictEqual(metacall('get_second', 5, 12), 12);
		});
	});

	describe('callback', () => {
		it('callback (py)', () => {
			const f = require('function.py');
			assert.notStrictEqual(f, undefined);

			// Passing callback as an argument
			const c = 5;
			assert.strictEqual(f.function_with_args((a, b) => {
				const result = a + b + c;
				assert.strictEqual(a, 2);
				assert.strictEqual(b, 3);
				assert.strictEqual(c, 5);
				console.log(`${a} + ${b} + ${c} = ${result}`);
				return result;
			}, 2, 3), 10);

			// Passing callback as a return value
			const callback = f.function_ret_lambda(10);
			assert.notStrictEqual(callback, undefined);
			assert.strictEqual(callback(3), 30);

			// Currying
			const currying = f.function_currying(10);
			assert.notStrictEqual(currying, undefined);
			assert.strictEqual(currying(2)(3), 60);

			// Currying more
			assert.strictEqual(f.function_currying_more(5)(4)(3)(2)(1), 120);

			// Receiving undefined from a function that returns nothing in Python
			assert.strictEqual(f.function_pass(), undefined);

			// Opaque pointer for class instances
			assert.strictEqual(f.function_capsule_method(f.function_capsule_new_class()), 'hello world');
			assert.strictEqual(f.function_capsule_method(f.function_capsule_new_class()), 'hello world'); // Check for function lifetime

			// Opaque pointer for class instances with callback
			assert.strictEqual(f.function_capsule_cb((klass) => f.function_capsule_method(klass)), 'hello world');
			assert.strictEqual(f.function_capsule_cb((klass) => f.function_capsule_method(klass)), 'hello world'); // Check for function lifetime

			// Double recursion
			const sum = (value, f) => value <= 0 ? 0 : value + f(value - 1, sum);
			assert.strictEqual(sum(5, f.function_sum), 15);
			assert.strictEqual(sum(5, f.function_sum), 15); // Check for function lifetime
			assert.strictEqual(f.function_sum(5, sum), 15);
			assert.strictEqual(f.function_sum(5, sum), 15); // Check for function lifetime

			// Factorial composition (@trgwii)
			// console.log("------------------------------------------------");
			// const fact = f.function_factorial(c => v => v <= 0 ? 1 : v);
			// console.log("------------------------------------------------");
			// assert.strictEqual(fact(1), 1);
			// console.log("------------------------------------------------");
			// assert.strictEqual(fact(2), 2);
			// console.log("------------------------------------------------");
			// assert.strictEqual(fact(3), 6);
			// console.log("------------------------------------------------");
			// assert.strictEqual(fact(50000), 2499950000);

			// console.log("------------------------------------------------");
			// const js_factorial = f.function_chain((x) => (n) => n == 0 ? 1 : n * x(x)(n - 1));
			// assert.notStrictEqual(js_factorial, undefined);
			// console.log("------------------------------------------------");
			// assert.strictEqual(js_factorial(5), 120);
			// console.log("------------------------------------------------");
			// assert.strictEqual(js_factorial(5), 120);

			// console.log("------------------------------------------------");
			// const py_factorial = f.function_chain(f.function_factorial);
			// assert.notStrictEqual(py_factorial, undefined);
			// console.log("------------------------------------------------");
			// assert.strictEqual(py_factorial(5), 120);
			// console.log("------------------------------------------------");
			// assert.strictEqual(py_factorial(5), 120);
		});
	});
});
