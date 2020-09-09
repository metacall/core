#!/usr/bin/env python3

import os
import sys
import json

abspath = os.path.dirname(os.path.abspath(__file__));
relpath = '../../source/ports/py_port/package';
path = os.path.normpath(os.path.join(abspath, relpath));

# Insert first in the sys path so we make sure we load the correct port
sys.path.insert(0, path);

from metacall import metacall, metacall_load_from_file, metacall_load_from_memory # TODO: , metacall_inspect

# Load mock
metacall_load_from_file('mock', ['test.mock']);

# Load test from memory
script = '''
from metacall import metacall
def test():
	print(metacall('three_str', 'a', 'b', 'c'));
	return metacall('my_empty_func_int');
'''

metacall_load_from_memory('py', script);

# Inspect contents (TODO)
# print(json.dumps(metacall_inspect(), indent = 4));
