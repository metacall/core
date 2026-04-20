#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

import os
import sys
import unittest
import asyncio

# Load metacall from Python Port path
abspath = os.path.dirname(os.path.abspath(__file__))
relpath = '..'
path = os.path.normpath(os.path.join(abspath, relpath))
sys.path.insert(0, path)

from metacall import (
	metacall,
	metacall_load_from_file,
	metacall_load_from_memory,
	metacall_inspect,
	metacall_await,
	MetaCallFunction
)


class TestMetaCallAwait(unittest.TestCase):
	"""Tests for metacall_await functionality."""

	@classmethod
	def setUpClass(cls):
		"""Load test scripts before running tests."""
		# Load Node.js async test script
		node_script = '''
async function async_add(a, b) {
	return a + b;
}

async function async_multiply(a, b) {
	return new Promise(resolve => {
		setTimeout(() => resolve(a * b), 10);
	});
}

async function async_delayed(value, delay_ms) {
	return new Promise(resolve => {
		setTimeout(() => resolve(value), delay_ms);
	});
}

async function async_return_object() {
	return { name: 'test', value: 42 };
}

async function async_return_array() {
	return [1, 2, 3, 4, 5];
}

async function async_return_null() {
	return null;
}

async function async_chain(value) {
	const step1 = await async_add(value, 10);
	const step2 = await async_multiply(step1, 2);
	return step2;
}

function sync_add(a, b) {
	return a + b;
}

module.exports = {
	async_add,
	async_multiply,
	async_delayed,
	async_return_object,
	async_return_array,
	async_return_null,
	async_chain,
	sync_add
};
'''
		result = metacall_load_from_memory('node', node_script)
		if not result:
			raise RuntimeError("Failed to load Node.js test script")

	def test_metacall_await_basic(self):
		"""Test basic async function call with await."""
		async def run_test():
			result = await metacall_await('async_add', 5, 3)
			self.assertEqual(result, 8)

		asyncio.run(run_test())

	def test_metacall_await_with_promise(self):
		"""Test async function that returns a Promise."""
		async def run_test():
			result = await metacall_await('async_multiply', 6, 7)
			self.assertEqual(result, 42)

		asyncio.run(run_test())

	def test_metacall_await_with_delay(self):
		"""Test async function with setTimeout delay."""
		async def run_test():
			result = await metacall_await('async_delayed', 'hello', 50)
			self.assertEqual(result, 'hello')

		asyncio.run(run_test())

	def test_metacall_await_returns_object(self):
		"""Test async function returning an object."""
		async def run_test():
			result = await metacall_await('async_return_object')
			self.assertIsInstance(result, dict)
			self.assertEqual(result['name'], 'test')
			self.assertEqual(result['value'], 42)

		asyncio.run(run_test())

	def test_metacall_await_returns_array(self):
		"""Test async function returning an array."""
		async def run_test():
			result = await metacall_await('async_return_array')
			self.assertIsInstance(result, list)
			self.assertEqual(result, [1, 2, 3, 4, 5])

		asyncio.run(run_test())

	def test_metacall_await_returns_null(self):
		"""Test async function returning null."""
		async def run_test():
			result = await metacall_await('async_return_null')
			self.assertIsNone(result)

		asyncio.run(run_test())

	def test_metacall_await_chained_calls(self):
		"""Test async function that chains other async calls."""
		async def run_test():
			# async_chain(5) -> async_add(5, 10) = 15 -> async_multiply(15, 2) = 30
			result = await metacall_await('async_chain', 5)
			self.assertEqual(result, 30)

		asyncio.run(run_test())

	def test_metacall_await_multiple_concurrent(self):
		"""Test multiple concurrent async calls."""
		async def run_test():
			results = await asyncio.gather(
				metacall_await('async_add', 1, 2),
				metacall_await('async_add', 3, 4),
				metacall_await('async_add', 5, 6)
			)
			self.assertEqual(results, [3, 7, 11])

		asyncio.run(run_test())

	def test_metacall_await_with_various_types(self):
		"""Test async calls with various argument types."""
		async def run_test():
			# Integer arguments
			result1 = await metacall_await('async_add', 100, 200)
			self.assertEqual(result1, 300)

			# Float arguments
			result2 = await metacall_await('async_add', 1.5, 2.5)
			self.assertEqual(result2, 4.0)

			# Negative numbers
			result3 = await metacall_await('async_add', -10, 5)
			self.assertEqual(result3, -5)

		asyncio.run(run_test())


class TestMetaCallFunction(unittest.TestCase):
	"""Tests for MetaCallFunction class."""

	@classmethod
	def setUpClass(cls):
		"""Load test scripts before running tests."""
		node_script = '''
async function wrapper_async_func(x) {
	return x * 2;
}

function wrapper_sync_func(x) {
	return x + 10;
}

async function wrapper_async_sum(a, b, c) {
	return a + b + c;
}

module.exports = {
	wrapper_async_func,
	wrapper_sync_func,
	wrapper_async_sum
};
'''
		metacall_load_from_memory('node', node_script)

	def test_metacall_function_creation_sync(self):
		"""Test creating a sync MetaCallFunction."""
		func = MetaCallFunction('wrapper_sync_func', is_async=False)

		self.assertEqual(func.name, 'wrapper_sync_func')
		self.assertFalse(func.is_async)
		self.assertEqual(func.__name__, 'wrapper_sync_func')

	def test_metacall_function_creation_async(self):
		"""Test creating an async MetaCallFunction."""
		func = MetaCallFunction('wrapper_async_func', is_async=True)

		self.assertEqual(func.name, 'wrapper_async_func')
		self.assertTrue(func.is_async)
		self.assertEqual(func.__name__, 'wrapper_async_func')

	def test_metacall_function_sync_call(self):
		"""Test calling a sync function through MetaCallFunction."""
		func = MetaCallFunction('wrapper_sync_func', is_async=False)
		result = func(5)
		self.assertEqual(result, 15)  # 5 + 10

	def test_metacall_function_async_call(self):
		"""Test calling an async function through MetaCallFunction."""
		func = MetaCallFunction('wrapper_async_func', is_async=True)

		async def run_test():
			result = await func(7)
			self.assertEqual(result, 14)  # 7 * 2

		asyncio.run(run_test())

	def test_metacall_function_async_call_method(self):
		"""Test using async_call method on async function."""
		func = MetaCallFunction('wrapper_async_func', is_async=True)

		async def run_test():
			result = await func.async_call(10)
			self.assertEqual(result, 20)  # 10 * 2

		asyncio.run(run_test())

	def test_metacall_function_async_call_on_sync(self):
		"""Test using async_call method on sync function."""
		func = MetaCallFunction('wrapper_sync_func', is_async=False)

		async def run_test():
			result = await func.async_call(5)
			self.assertEqual(result, 15)  # 5 + 10

		asyncio.run(run_test())

	def test_metacall_function_repr(self):
		"""Test string representation of MetaCallFunction."""
		sync_func = MetaCallFunction('test_sync', is_async=False)
		async_func = MetaCallFunction('test_async', is_async=True)

		self.assertIn('test_sync', repr(sync_func))
		self.assertIn('is_async=False', repr(sync_func))
		self.assertIn('test_async', repr(async_func))
		self.assertIn('is_async=True', repr(async_func))

	def test_metacall_function_str(self):
		"""Test str() of MetaCallFunction."""
		sync_func = MetaCallFunction('test_sync', is_async=False)
		async_func = MetaCallFunction('test_async', is_async=True)

		self.assertIn('sync', str(sync_func))
		self.assertIn('async', str(async_func))

	def test_metacall_function_with_multiple_args(self):
		"""Test MetaCallFunction with multiple arguments."""
		func = MetaCallFunction('wrapper_async_sum', is_async=True)

		async def run_test():
			result = await func(1, 2, 3)
			self.assertEqual(result, 6)

		asyncio.run(run_test())


class TestAsyncImportDetection(unittest.TestCase):
	"""Tests for automatic async function detection during import."""

	@classmethod
	def setUpClass(cls):
		"""Load test script with both sync and async functions."""
		node_script = '''
async function import_async_double(x) {
	return x * 2;
}

function import_sync_triple(x) {
	return x * 3;
}

async function import_async_greet(name) {
	return 'Hello, ' + name + '!';
}

module.exports = {
	import_async_double,
	import_sync_triple,
	import_async_greet
};
'''
		metacall_load_from_memory('node', node_script)

	def test_inspect_shows_async_property(self):
		"""Test that inspect returns async property for functions."""
		metadata = metacall_inspect()

		# Find our test functions in the metadata
		found_async = False
		found_sync = False

		for loader_key in metadata:
			for handle in metadata[loader_key]:
				for func in handle.get('scope', {}).get('funcs', []):
					if func['name'] == 'import_async_double':
						self.assertTrue(func.get('async', False),
							"import_async_double should be marked as async")
						found_async = True
					elif func['name'] == 'import_sync_triple':
						self.assertFalse(func.get('async', False),
							"import_sync_triple should not be marked as async")
						found_sync = True

		self.assertTrue(found_async, "Did not find import_async_double in metadata")
		self.assertTrue(found_sync, "Did not find import_sync_triple in metadata")


class TestAwaitEdgeCases(unittest.TestCase):
	"""Tests for edge cases in await functionality."""

	@classmethod
	def setUpClass(cls):
		"""Load test script with edge case functions."""
		node_script = '''
async function edge_no_args() {
	return 'no args result';
}

async function edge_many_args(a, b, c, d, e) {
	return a + b + c + d + e;
}

async function edge_long_string() {
	return 'a'.repeat(10000);
}

async function edge_large_array() {
	return Array.from({length: 1000}, (_, i) => i);
}

async function edge_deep_object() {
	return {
		level1: {
			level2: {
				level3: {
					value: 'deep'
				}
			}
		}
	};
}

module.exports = {
	edge_no_args,
	edge_many_args,
	edge_long_string,
	edge_large_array,
	edge_deep_object
};
'''
		metacall_load_from_memory('node', node_script)

	def test_await_no_args(self):
		"""Test async function with no arguments."""
		async def run_test():
			result = await metacall_await('edge_no_args')
			self.assertEqual(result, 'no args result')

		asyncio.run(run_test())

	def test_await_many_args(self):
		"""Test async function with many arguments."""
		async def run_test():
			result = await metacall_await('edge_many_args', 1, 2, 3, 4, 5)
			self.assertEqual(result, 15)

		asyncio.run(run_test())

	def test_await_long_string(self):
		"""Test async function returning long string."""
		async def run_test():
			result = await metacall_await('edge_long_string')
			self.assertEqual(len(result), 10000)
			self.assertTrue(all(c == 'a' for c in result))

		asyncio.run(run_test())

	def test_await_large_array(self):
		"""Test async function returning large array."""
		async def run_test():
			result = await metacall_await('edge_large_array')
			self.assertEqual(len(result), 1000)
			self.assertEqual(result[0], 0)
			self.assertEqual(result[999], 999)

		asyncio.run(run_test())

	def test_await_deep_object(self):
		"""Test async function returning deeply nested object."""
		async def run_test():
			result = await metacall_await('edge_deep_object')
			self.assertEqual(result['level1']['level2']['level3']['value'], 'deep')

		asyncio.run(run_test())


class TestAwaitConcurrency(unittest.TestCase):
	"""Tests for concurrent await operations."""

	@classmethod
	def setUpClass(cls):
		"""Load test script for concurrency tests."""
		node_script = '''
let counter = 0;

async function concurrent_increment() {
	counter++;
	await new Promise(r => setTimeout(r, 10));
	return counter;
}

async function concurrent_get_counter() {
	return counter;
}

async function concurrent_reset() {
	counter = 0;
	return true;
}

async function concurrent_slow(id, delay) {
	await new Promise(r => setTimeout(r, delay));
	return id;
}

module.exports = {
	concurrent_increment,
	concurrent_get_counter,
	concurrent_reset,
	concurrent_slow
};
'''
		metacall_load_from_memory('node', node_script)

	def test_sequential_awaits(self):
		"""Test multiple sequential await calls."""
		async def run_test():
			await metacall_await('concurrent_reset')

			result1 = await metacall_await('concurrent_increment')
			result2 = await metacall_await('concurrent_increment')
			result3 = await metacall_await('concurrent_increment')

			# Sequential calls should increment properly
			self.assertEqual(result3, 3)

		asyncio.run(run_test())

	def test_gather_multiple_awaits(self):
		"""Test concurrent awaits with asyncio.gather."""
		async def run_test():
			# Run multiple slow operations concurrently
			results = await asyncio.gather(
				metacall_await('concurrent_slow', 'a', 30),
				metacall_await('concurrent_slow', 'b', 20),
				metacall_await('concurrent_slow', 'c', 10)
			)

			# All results should complete
			self.assertEqual(set(results), {'a', 'b', 'c'})

		asyncio.run(run_test())

	def test_task_creation(self):
		"""Test creating asyncio Tasks from await futures."""
		async def run_test():
			task1 = asyncio.create_task(metacall_await('concurrent_slow', 1, 20))
			task2 = asyncio.create_task(metacall_await('concurrent_slow', 2, 10))

			result2 = await task2
			result1 = await task1

			self.assertEqual(result1, 1)
			self.assertEqual(result2, 2)

		asyncio.run(run_test())


class TestMetaCallFunctionAdvanced(unittest.TestCase):
	"""Advanced tests for MetaCallFunction."""

	@classmethod
	def setUpClass(cls):
		"""Load test script."""
		node_script = '''
async function adv_process(data) {
	return { processed: true, input: data };
}

function adv_transform(value) {
	return value.toUpperCase();
}

module.exports = { adv_process, adv_transform };
'''
		metacall_load_from_memory('node', node_script)

	def test_metacall_function_doc(self):
		"""Test MetaCallFunction has proper documentation."""
		func = MetaCallFunction('adv_process', is_async=True)
		self.assertIn('adv_process', func.__doc__)
		self.assertIn('async', func.__doc__)

	def test_metacall_function_callable(self):
		"""Test MetaCallFunction is callable."""
		func = MetaCallFunction('adv_transform', is_async=False)
		self.assertTrue(callable(func))

	def test_metacall_function_as_higher_order(self):
		"""Test using MetaCallFunction in higher-order scenarios."""
		sync_func = MetaCallFunction('adv_transform', is_async=False)

		# Use in map-like scenario
		inputs = ['hello', 'world']
		results = [sync_func(x) for x in inputs]

		self.assertEqual(results, ['HELLO', 'WORLD'])

	def test_metacall_function_properties(self):
		"""Test MetaCallFunction property access."""
		func = MetaCallFunction('adv_process', is_async=True)

		# Properties should be accessible
		self.assertEqual(func.name, 'adv_process')
		self.assertTrue(func.is_async)


if __name__ == '__main__':
	unittest.main()
