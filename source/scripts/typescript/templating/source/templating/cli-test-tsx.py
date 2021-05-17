#!/usr/bin/env python3

# This script is here in order to reproduce the bug of python-tsx-example

def main():
	import sys
	import os

	# Insert Python Port folder first in the system path list
	sys.path.insert(0, os.environ['PROJECT_METACALL_PORTS_DIRECTORY'])

	import metacall
	from templating.tsx import hello
	print(hello('World'))

# Run the main
main();
