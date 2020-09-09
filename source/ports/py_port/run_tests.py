#!/usr/bin/env python3

import unittest
import os

loader = unittest.TestLoader()
abspath = os.path.dirname(os.path.abspath(__file__));
path = os.path.normpath(os.path.join(abspath, 'test'));
suite = loader.discover(path)

runner = unittest.TextTestRunner()
runner.run(suite)
