#!/usr/bin/python3.5

import os
import sys

port_path = os.environ.get('PORT_LIBRARY_PATH')

if not port_path:
	port_path = '/usr/local/lib'

sys.path.append(port_path);

try:
	from _py_port import *
except ImportError:
	try:
		from _py_portd import *
	except ImportError:
		pass
