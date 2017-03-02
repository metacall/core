#!/usr/bin/python3.4

from callback_metacall import callback_host

def hello_world(text: str) -> int:

	print('Python hello_world: ' + text);

	return callback_host(text);
