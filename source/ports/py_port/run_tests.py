#!/usr/bin/env python3

import unittest
import os

def main():
	loader = unittest.TestLoader()
	abspath = os.path.dirname(os.path.abspath(__file__));
	path = os.path.normpath(os.path.join(abspath, 'test'));
	suite = loader.discover(path)

	runner = unittest.TextTestRunner()
	result = runner.run(suite)

	return "Tests passed without errors" if result.errors > 0 or result.failures else "";
