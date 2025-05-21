#!/usr/bin/env python3

import unittest
import os
import sys

def main():
	loader = unittest.TestLoader()
	abspath = os.path.dirname(os.path.abspath(__file__))
	path = os.path.normpath(os.path.join(abspath, 'test'))
	suite = loader.discover(path)

	runner = unittest.TextTestRunner()
	result = runner.run(suite)

	if len(result.errors) + len(result.failures) == 0:
		return 'Tests passed without errors'
	else:
		exit(1)

if __name__ == "__main__":
    main()
