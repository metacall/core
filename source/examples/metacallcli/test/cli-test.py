#!/usr/bin/env python3

import os
import sys

abspath = os.path.dirname(os.path.abspath(__file__));
relpath = '../../../ports/py_port/package';

sys.path.append(os.path.normpath(os.path.join(abspath, relpath)));

from metacall import metacall, metacall_load_from_file

metacall_load_from_file('mock', ['test.mock']);

def test():
	return metacall('three_str', 'a', 'b', 'c');
