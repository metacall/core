#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
import re

# Append environment variable or default install path when building manually (TODO: Cross-platform paths)
sys.path.append(os.environ.get('PORT_LIBRARY_PATH', os.path.join(os.path.sep, 'usr', 'local', 'lib')));

# Find is MetaCall is installed as a distributable tarball (TODO: Cross-platform paths)
rootdir = os.path.join(os.path.sep, 'gnu', 'store')
regex = re.compile('.*-metacall-.*')

for root, dirs, _ in os.walk(rootdir):
	for folder in dirs:
		if regex.match(folder) and not folder.endswith('R'):
			sys.path.append(os.path.join(rootdir, folder, 'lib'))

try:
	from _py_port import * # TODO: Import only the functions that will be exported
except ImportError as e:
	try:
		print('Error when importing MetaCall Python Port:', e);
		from _py_portd import * # TODO: Import only the functions that will be exported
		print('MetaCall Python Port Debug Imported');
	except ImportError as e:
		print('\x1b[31m\x1b[1m', 'You do not have MetaCall installed or we cannot find it (', e, ')\x1b[0m');
		print('\x1b[33m\x1b[1m', 'If you do not have it installed, you have three options:', '\x1b[0m');
		print('\x1b[1m', '	1) Go to https://github.com/metacall/install and install it.', '\x1b[0m');
		print('\x1b[1m', '	2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.', '\033[0m');
		print('\x1b[1m', '	3) Be a x10 programmer and compile it by yourself, then define the install folder (if it is different from the default /usr/local/lib) in os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m');
		print('\x1b[33m\x1b[1m', 'If you have it installed in an non-standard folder, please define os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m');
		pass

# TODO: Monkey patch
