#!/usr/bin/env python3

import os
import sys
import unittest

# Load metacall from Python Port path
abspath = os.path.dirname(os.path.abspath(__file__))
relpath = '..'
path = os.path.normpath(os.path.join(abspath, relpath))

# Insert first in the sys path so we make sure we load the correct port
sys.path.insert(0, path)

from metacall import *

class py_port_test(unittest.TestCase):

	# MetaCall (Mock)
	def test_mock(self):
		try:
			result = metacall_load_from_file('mock', ['sometestmock.mock'])
			if result != True:
				self.skipTest('Mock loader not available')
		except:
			self.skipTest('Mock loader not available')

		self.assertEqual(metacall('three_str', 'a', 'b', 'c'), 'Hello World')

	# MetaCall (Python from memory)
	def test_python_memory(self):
		script = '#!/usr/bin/env python3\ndef inline_multiply_mem(left, right):\n\treturn left * right\n'

		self.assertEqual(metacall_load_from_memory('py', script), True)

		self.assertEqual(metacall('inline_multiply_mem', 2, 2), 4)

	# MetaCall (Python)
	def test_python(self):
		import example

		self.assertEqual(example.hello(), None)

		self.assertEqual(example.multiply(5, 7), 35)

		self.assertEqual(metacall_load_from_file('py', ['helloworld.py']), True)

		self.assertEqual(metacall('s_sum', 5, 5), 10)

	# MetaCall (Ruby)
	def test_ruby(self):
		# Check if Ruby loader is available
		try:
			result = metacall_load_from_file('rb', ['second.rb'])
			if result != True:
				self.skipTest('Ruby loader not available')
		except:
			self.skipTest('Ruby loader not available')

		from second.rb import get_second, get_second_untyped

		self.assertEqual(get_second(34, 22), 22)

		self.assertEqual(get_second_untyped(34, 22), 22)

		self.assertEqual(metacall_load_from_file('rb', ['hello.rb']), True)

		self.assertEqual(metacall('say_null'), None)

		self.assertEqual(metacall('say_multiply', 3, 4), 12)

		self.assertEqual(metacall('say_hello', 'world'), 'Hello world!')

	# MetaCall (NodeJS)
	def test_nodejs(self):
		# Check if NodeJS loader is available
		try:
			result = metacall_load_from_file('node', ['derpyramda.js'])
			if result != True:
				self.skipTest('NodeJS loader not available')
		except:
			self.skipTest('NodeJS loader not available')

		from derpyramda.js import add, subtract, flip

		self.assertEqual(add(3, 4), 7.0)
		self.assertEqual(subtract(5, 4), 1.0)

		# TODO:
		# The current code does the following:
		#	flip is a Python function which wraps a NodeJS function,
		#	when calling to the function in NodeJS, the call gets enqueued
		#	in order to make a thread safe call into the V8 thread. Once this
		#	happens, flip calls back into a Python lambda (in the first case),
		#	or function (wrapping subtract) in the second case. As the Python GIL is
		#	already acquired by the first thread, the main thread which did the call,
		#	different from the V8 thread, the GIL gets aquired in the new thread.
		#
		#		|          Python Thread          |        V8 Thread       |
		#		|----------------------------------------------------------|
		#		| GIL lock                        |                        |
		#		|    Call flip                    |                        |
		#		|        Enqueue call in V8       |                        |
		#		|        Wait for V8              |                        |
		#		|                                 | Execute flip           |
		#		|                                 |    Call lambda         |
		#		|                                 |        GIL lock        |
		#
		# For avoiding extra work when executing calls in the same thread, this is the mechanism that we can use:
		#	1) Get the thread id of the thread where Python was launched, similarly to this:
		#		https://github.com/metacall/core/blob/9ad4ed8964a53e30d8ab478a53122c396d705cdd/source/loaders/node_loader/source/node_loader_impl.cpp#L3158
		#	2) Check if the current thread is the same as where Python interpreter was launched and check against this:
		#		PyGILState_Check: https://python.readthedocs.io/en/latest/c-api/init.html#c.PyGILState_Check
		#
		self.assertEqual(flip(lambda x, y: x - y)(5, 4), -1.0)
		self.assertEqual(flip(subtract)(5, 4), -1.0)

	# MetaCall (Rust)
	def test_rust(self):
		if os.environ.get('OPTION_BUILD_LOADERS_RS') == None: return

		from basic.rs import add, add_float, string_len, new_string

		self.assertEqual(add(34, 22), 56)

		self.assertEqual(add_float(34.0, 22.0), 56.0)

		# FIXME: python will get a string with random suffix
		# s = new_string(3)
		# self.assertEqual(s, 'get number 3')

		self.assertEqual(string_len('world'), 5)

		metacall_load_from_memory("rs", """
fn return_string() -> String {
	println!("return hello world");
    String::from("hello world")
}

fn new_string2(idx: i32) -> String {
    format!("get number {idx}")
}
""")
		self.assertEqual(metacall('new_string2', 5), 'get number 5')

		# FIXME: println inside rust fails to log anything
		self.assertEqual(metacall("return_string"), "hello world")

if __name__ == '__main__':
	unittest.main()
