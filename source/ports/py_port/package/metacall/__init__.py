#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

import os
import sys

sys.path.append(os.environ.get('PORT_LIBRARY_PATH', '/usr/local/lib'));

try:
	from _py_port import * # TODO: Import only the functions that will be exported
except ImportError:
	try:
		from _py_portd import * # TODO: Import only the functions that will be exported
	except ImportError: # TODO: Remove this (even with the build + install or pip install, runtime should be installed before reaching this)
		pass #	This never must happen, the import error must be raised explaining that the port library (or runtime) is not installed
