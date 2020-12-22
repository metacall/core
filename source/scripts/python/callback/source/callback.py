#!/usr/bin/env python3

from callback_metacall import callback_host, callback_ruby

def hello_world(text: str) -> int:

	print('Python hello_world: ' + text)

	return callback_host(text)

def hello_ruby(left: int, right: int) -> int:

	print('Python hello_ruby: ' + str(left) + ' * ' + str(right))

	return callback_ruby(left, right)
