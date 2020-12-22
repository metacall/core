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

	# MetaCall (NodeJS)
	def test_nodejs(self):
		from derpyramda.js import add, subtract, flip

		self.assertEqual(add(3, 4), 7.0);
		self.assertEqual(subtract(5, 4), 1.0);

		# TODO:
		# The following tests deadlock because this:
		#	flip is a Python function which wraps a NodeJS function,
		#	when calling to the function in NodeJS, the call gets enqueued
		#	in order to make a thread safe call into the V8 thread. Once this
		#	happens, flip calls back into a Python lambda (in the first case),
		#	or function (wrapping subtract) in the second case. As the Python GIL is
		#	already acquired by the first thread, the main thread which did the call,
		#	different from the V8 thread, it deadlocks when trying to acquire it again in
		#	the V8 thread.
		#
		#		|          Python Thread          |           V8 Thread          |
		#		|----------------------------------------------------------------|
		#		| GIL lock                        |                              |
		#		|    Call flip                    |                              |
		#		|        Enqueue call in V8       |                              |
		#		|        Wait for V8              |                              |
		#		|                                 | Execute flip                 |
		#		|                                 |    Call lambda               |
		#		|                                 |        GIL lock (deadlock)   |
		#
		# There may be different solutions to this problem, one way to avoid this, which I
		# already thought about it, and it may also improve performance, is to detect when
		# we are passing a NodeJS function to flip, and "unwrap" the substract function in
		# order to pass it directly to V8 thread, this will work for the second case, but not
		# the first case. Another option which will work for all cases is to make all calls async,
		# and always force the end user to develop with async calls. I think it is a very interesting
		# option because it fits well in a FaaS model, but I am not sure what implications can have
		# for other existing kind of applications or for the simplicity and easy of usage.
		# Probably there is an alternative option, for example detecting deadlocks on the fly and
		# try to solve them in some way, maybe having another interpreter instance, or some co-routine
		# like mechanism, I don't know.
		#
		# For tracking deadlocks, this is the mechanism that we can use:
		#	1) Get the thread id of the thread where Python was launched, similarly to this:
		#		https://github.com/metacall/core/blob/9ad4ed8964a53e30d8ab478a53122c396d705cdd/source/loaders/node_loader/source/node_loader_impl.cpp#L3158
		#	2) Check if the current thread is the same as where Python interpreter was launched and check against this:
		#		PyGILState_Check: https://python.readthedocs.io/en/latest/c-api/init.html#c.PyGILState_Check
		# This methodology could be use to implement reentrant calls too, but the GIL already has an internal counter
		# for tracking how many times the GIL has been acquired so there is no need for that.
		#
		# self.assertEqual(flip(lambda x, y: x - y)(5, 4), -1.0);
		# self.assertEqual(flip(subtract)(5, 4), -1.0);

if __name__ == '__main__':
	unittest.main()
