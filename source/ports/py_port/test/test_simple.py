#!/usr/bin/env python3

import os
import sys
import unittest

# Load metacall from Python Port path
abspath = os.path.dirname(os.path.abspath(__file__));
relpath = '..';
path = os.path.normpath(os.path.join(abspath, relpath));

# Insert first in the sys path so we make sure we load the correct port
sys.path.insert(0, path);

from metacall import *

class py_port_test(unittest.TestCase):

	# MetaCall (Mock)
	def test_mock(self):
		self.assertEqual(metacall_load_from_file('mock', ['sometestmock.mock']), True);

		self.assertEqual(metacall('three_str', 'a', 'b', 'c'), 'Hello World');

		# Monkey patch
		import sometestmock

		self.assertEqual(sometestmock.my_empty_func_int(), 1234);
		self.assertEqual(sometestmock.three_str('a', 'b', 'c'), 'Hello World');

		# Monkey patch without preload and with extension
		import othertest.mock

		self.assertEqual(othertest.my_empty_func_int(), 1234);

		# # TODO: Monkey patch without preload and without extension (it works but some runtimes like C# Loader explode when trying to load it)
		# import thismoduledoesnotexist

		# self.assertEqual(thismoduledoesnotexist.my_empty_func_int(), 1234)

	# MetaCall (Python from memory)
	def test_python_memory(self):
		script = '#!/usr/bin/env python3\ndef inline_multiply_mem(left, right):\n\treturn left * right\n';

		self.assertEqual(metacall_load_from_memory('py', script), True);

		self.assertEqual(metacall('inline_multiply_mem', 2, 2), 4);

	# MetaCall (Python)
	def test_python(self):
		self.assertEqual(metacall_load_from_file('py', ['example.py']), True);

		self.assertEqual(metacall('hello'), None);

		self.assertEqual(metacall('multiply', 5, 7), 35);

	# MetaCall (Ruby)
	def test_ruby(self):
		self.assertEqual(metacall_load_from_file('rb', ['hello.rb']), True);

		self.assertEqual(metacall('say_null'), None);

		self.assertEqual(metacall('say_multiply', 3, 4), 12);

		self.assertEqual(metacall('say_hello', 'world'), 'Hello world!');

if __name__ == '__main__':
	unittest.main()
