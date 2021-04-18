/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

const {
	metacall,
	metacall_load_from_file,
	metacall_load_from_file_export,
	metacall_load_from_memory,
	metacall_load_from_memory_export,
	metacall_handle,
	metacall_inspect,
	metacall_logs
} = require('../index.js');

describe('metacall', () => {
	describe('defined', () => {
		it('functions metacall and metacall_load_from_file must be defined', () => {
			assert.notStrictEqual(metacall, undefined);
			assert.notStrictEqual(metacall_load_from_memory, undefined);
			assert.notStrictEqual(metacall_load_from_file, undefined);
			assert.notStrictEqual(metacall_load_from_memory_export, undefined);
			assert.notStrictEqual(metacall_load_from_file_export, undefined);
			assert.notStrictEqual(metacall_inspect, undefined);
			assert.notStrictEqual(metacall_logs, undefined);
		});
	});

	// TODO: This fails in NodeJS 15.x because the error message is slightly different
	/*
	describe('fail', () => {
		it('require', () => {
			assert.throws(() => { require('./asd.invalid') }, new Error('Cannot find module \'./asd.invalid\''));
			// TODO: Improve error messages
			assert.throws(() => { require('./asd.py') }, new Error('MetaCall could not load from file'));
			assert.throws(() => { require('./asd.rb') }, new Error('MetaCall could not load from file'));
			assert.throws(() => { require('./asd.cs') }, new Error('MetaCall could not load from file'));
			assert.throws(() => { require('./asd.ts') }, new Error('MetaCall could not load from file'));
			assert.throws(() => { require('./asd.tsx') }, new Error('MetaCall could not load from file'));
		});
	});
	*/

	describe('load', () => {
		it('metacall_load_from_file (py)', () => {
			assert.strictEqual(metacall_load_from_file('py', [ 'helloworld.py' ] ), undefined);

			const script = metacall_handle('py', 'helloworld.py');
			assert.notStrictEqual(script, undefined);
			assert.strictEqual(script.name, 'helloworld.py');
		});
		it('metacall_load_from_file_export (py)', () => {
			const handle = metacall_load_from_file_export('py', [ 'ducktype.py' ] );
			assert.notStrictEqual(handle, undefined);
			assert.strictEqual(handle.sum(1, 2), 3);

			// TODO: Need a way to test the symbol is not globally defined.
			//assert.strictEqual(metacall('sum'), undefined);
		});
		it('metacall_load_from_file (rb)', () => {
			assert.strictEqual(metacall_load_from_file('rb', [ 'ducktype.rb' ]), undefined);

			const script = metacall_handle('rb', 'ducktype.rb');
			assert.notStrictEqual(script, undefined);
			assert.strictEqual(script.name, 'ducktype.rb');
		});
		it('metacall_load_from_memory (py)', () => {
			assert.strictEqual(metacall_load_from_memory('py', 'def py_memory():\n\treturn 4;\n'), undefined);
			assert.strictEqual(metacall('py_memory'), 4.0);
		});
		it('metacall_load_from_memory_export (py)', () => {
			const handle = metacall_load_from_memory_export('py', 'def py_memory_export():\n\treturn 6;\n');
			assert.notStrictEqual(handle, undefined);
			assert.strictEqual(handle.py_memory_export(), 6.0);

			// TODO: Need a way to test the symbol is not globally defined.
			//assert.strictEqual(metacall('py_memory_export'), undefined);
		});
		// Cobol tests are conditional (in order to pass CI/CD)
		if (process.env['OPTION_BUILD_LOADERS_COB']) {
			it('metacall_load_from_file (cob)', () => {
				assert.strictEqual(metacall_load_from_file('cob', [ 'say.cob' ]), undefined);

				const script = metacall_handle('cob', 'say.cob');
				assert.notStrictEqual(script, undefined);
				assert.strictEqual(script.name, 'say.cob');

				assert.strictEqual(metacall('say', 'Hello, ', 'world!'), 0);
			});
		}
		it('require (mock)', () => {
			// TODO: Both methods work, should we disable the commented out style to be NodeJS compilant?
			// const asd = require('asd.mock');
			const asd = require('./asd.mock');
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
		it('require (py)', () => {
			// TODO: Both methods work, should we disable the commented out style to be NodeJS compilant?
			// const example = require('example.py');
			const example = require('./example.py');
			assert.notStrictEqual(example, undefined);
			assert.strictEqual(example.multiply(2, 2), 4);
			assert.strictEqual(example.divide(4.0, 2.0), 2.0);
			assert.strictEqual(example.sum(2, 2), 4);
			assert.strictEqual(example.strcat('2', '2'), '22');
			assert.deepStrictEqual(example.return_array(), [1, 2, 3]);
			assert.deepStrictEqual(example.return_same_array([1, 2, 3]), [1, 2, 3]);
		});
		it('require (py module)', () => {
			// This code loads directly a module without extension from Python
			const { escape } = require('html');
			assert.notStrictEqual(escape, undefined);
			assert.strictEqual(escape('<html></html>'), '&lt;html&gt;&lt;/html&gt;');
		});
		// TODO: This fails, not sure why
		/*
		it('require (py submodule)', () => {
			// This code loads directly a module without extension from Python
			const { py_encode_basestring_ascii } = require('json.encoder');
			assert.notStrictEqual(py_encode_basestring_ascii, undefined);
			assert.strictEqual(py_encode_basestring_ascii('asd'), '"asd"');
		});
		*/
		it('require (rb)', () => {
			// TODO: Both methods work, should we disable the commented out style to be NodeJS compilant?
			// const cache = require('cache.rb');
			const cache = require('./cache.rb');
			assert.notStrictEqual(cache, undefined);
			assert.strictEqual(cache.cache_set('asd', 'efg'), undefined);
			assert.strictEqual(cache.cache_get('asd'), 'efg');
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
		it('metacall (py)', () => {
			assert.strictEqual(metacall('s_sum', 2, 2), 4);
		});
		it('metacall (rb)', () => {
			assert.strictEqual(metacall('get_second', 5, 12), 12);
		});
	});

	// TODO: This fails because classes are not implemented in the NodeJS loader
	/*
	describe('callback', () => {
		it('callback (py)', () => {
			const py_f = require('function.py');
			assert.notStrictEqual(py_f, undefined);

			// Passing callback as an argument
			const c = 5;
			assert.strictEqual(py_f.function_with_args((a, b) => {
				const result = a + b + c;
				assert.strictEqual(a, 2);
				assert.strictEqual(b, 3);
				assert.strictEqual(c, 5);
				console.log(`${a} + ${b} + ${c} = ${result}`);
				return result;
			}, 2, 3), 10);

			// Passing callback as a return value
			const callback = py_f.function_ret_lambda(10);
			assert.notStrictEqual(callback, undefined);
			assert.strictEqual(callback(3), 30);

			// Currying
			const currying = py_f.function_currying(10);
			assert.notStrictEqual(currying, undefined);
			assert.strictEqual(currying(2)(3), 60);

			// Currying more
			assert.strictEqual(py_f.function_currying_more(5)(4)(3)(2)(1), 120);

			// Receiving undefined from a function that returns nothing in Python
			assert.strictEqual(py_f.function_pass(), undefined);

			/* TODO: After the refactor of class/object support the following tests do not pass */
			/* Now the class returned by Python is threated as a TYPE_CLASS instead of a TYPE_PTR */
			/* Refactor this when there is support for class in NodeJS Loader */

			// Class test
			/*
			assert.strictEqual(py_f.function_myclass_method(py_f.function_myclass_new_class()), 'hello world');
			assert.strictEqual(py_f.function_myclass_method(py_f.function_myclass_new_class()), 'hello world'); // Check for function lifetime
			*/

			// Class test with callback
			/*
			assert.strictEqual(py_f.function_myclass_cb((klass) => py_f.function_myclass_method(klass)), 'hello world');
			assert.strictEqual(py_f.function_myclass_cb((klass) => py_f.function_myclass_method(klass)), 'hello world'); // Check for function lifetime
			*/
	/*

			// Double recursion
			const sum = (value, f) => value <= 0 ? 0 : value + f(value - 1, sum);
			assert.strictEqual(sum(5, py_f.function_sum), 15);
			assert.strictEqual(sum(5, py_f.function_sum), 15);
			assert.strictEqual(sum(5, py_f.function_sum), 15);
			assert.strictEqual(sum(5, py_f.function_sum), 15); // Check for function lifetime
			assert.strictEqual(sum(10, py_f.function_sum), 55);
			assert.strictEqual(sum(60, py_f.function_sum), 1830);
			assert.strictEqual(sum(160, py_f.function_sum), 12880); // Big numbers
			assert.strictEqual(py_f.function_sum(5, sum), 15);
			assert.strictEqual(py_f.function_sum(5, sum), 15);
			assert.strictEqual(py_f.function_sum(5, sum), 15);
			assert.strictEqual(py_f.function_sum(5, sum), 15); // Check for function lifetime
			assert.strictEqual(py_f.function_sum(10, sum), 55);
			assert.strictEqual(py_f.function_sum(60, sum), 1830);
			assert.strictEqual(py_f.function_sum(160, sum), 12880); // Big numbers

			// Factorial composition (@trgwii)
			const fact = py_f.function_factorial(c => v => v <= 0 ? 1 : v);
			assert.strictEqual(fact(1), 1);
			assert.strictEqual(fact(2), 2);
			assert.strictEqual(fact(3), 6);

			const js_function_chain = function(x) {
				return function(n) {
					console.log('------------------ js chain', n);
					console.log('------------------ js chain pre x() call', x.toString().slice(0, 12), '...');
					const result = x(x)(n);
					console.log('------------------ js chain post x() call', x.toString().slice(0, 12), '...');
					return result;
				};
			};
			const js_factorial_impl = function(x) {
				return function(n) {
					console.log('------------------ js factorial', n);
					if (n == 0) {
						console.log('------------------ js factorial case base');
						return 1;
					} else {
						console.log('------------------ js factorial pre x() call', x.toString().slice(0, 12), '...');
						const result = n * x(x)(n - 1);
						console.log('------------------ js factorial post x() call', x.toString().slice(0, 12), '...');
						return result;
					}
				};
			}

			const js_js_factorial = js_function_chain(js_factorial_impl);
			assert.notStrictEqual(js_js_factorial, undefined);
			assert.strictEqual(js_js_factorial(5), 120);
			assert.strictEqual(js_js_factorial(5), 120);

			const py_py_factorial = py_f.function_chain(py_f.py_function_factorial);
			assert.notStrictEqual(py_py_factorial, undefined);
			assert.strictEqual(py_py_factorial(5), 120);
			assert.strictEqual(py_py_factorial(5), 120);

			const py_js_factorial = py_f.function_chain(js_factorial_impl);
			assert.notStrictEqual(py_js_factorial, undefined);
			assert.strictEqual(py_js_factorial(5), 120);
			assert.strictEqual(py_js_factorial(5), 120);

			const py_factorial = py_f.function_chain(py_f.function_factorial);
			assert.notStrictEqual(py_factorial, undefined);
			assert.strictEqual(py_factorial(5), 120);
			assert.strictEqual(py_factorial(5), 120);
		});
	});
	*/
});
