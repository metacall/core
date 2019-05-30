#!/usr/bin/python3.5

import os
import sys

# TODO: This is a nasty mock. It only will work if python port library (binary) is already installed and the
# environment variable correctly set up. This must be removed when metacall is correctly distributed
# and we can automate all distributions for all architectures, operative systems and all ports for all languages
sys.path.append(os.environ['PORT_LIBRARY_PATH']);

try:
	from _py_port import *
except ImportError:
	from _py_portd import *
