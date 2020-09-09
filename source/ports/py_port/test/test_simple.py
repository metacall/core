#!/usr/bin/env python3

import os
import sys
import unittest

abspath = os.path.dirname(os.path.abspath(__file__));
relpath = '..';
path = os.path.normpath(os.path.join(abspath, relpath));

print('-----------------------------------------------------')
print(path)
print('----------3#################-------------------------------------------')

# Insert first in the sys path so we make sure we load the correct port
sys.path.insert(0, path);

from metacall import *

class py_port_test(unittest.TestCase):

	# TODO

	# # MetaCall (Mock)
	# def test_mock(self):
	# 	self.assertEqual(metacall_load_from_file('mock', ['test.mock']), True);

	# 	self.assertEqual(metacall('three_str', 'a', 'b', 'c'), 'Hello World');

	# # MetaCall (Python from memory)
	# def test_python_memory(self):
	# 	script = '#!/usr/bin/env python3\ndef inline_multiply_mem(left, right):\n\treturn left * right\n';

	# 	self.assertEqual(metacall_load_from_memory('py', script), True);

	# 	self.assertEqual(metacall('inline_multiply_mem', 2, 2), 4);

	# # MetaCall (Python)
	# def test_python(self):
	# 	self.assertEqual(metacall_load_from_file('py', ['example.py']), True);

	# 	self.assertEqual(metacall('hello'), None);

	# 	self.assertEqual(metacall('multiply', 5, 7), 35);

	# MetaCall (Ruby)
	def test_ruby(self):
		self.assertEqual(metacall_load_from_file('rb', ['hello.rb']), True);

		self.assertEqual(metacall('say_null'), None);

		self.assertEqual(metacall('say_multiply', 3, 4), 12);

		self.assertEqual(metacall('say_hello', 'world'), 'Hello world!');

if __name__ == '__main__':
	unittest.main()
