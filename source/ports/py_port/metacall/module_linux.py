#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

def metacall_module_load():
	# Append environment variable or default install path when building manually
	sys.path.append(os.environ.get('LOADER_LIBRARY_PATH', os.path.join(os.path.sep, 'usr', 'local', 'lib')))

	# Find is MetaCall is installed as a distributable tarball
	rootdir = os.path.join(os.path.sep, 'gnu', 'store')
	regex = re.compile('.*-metacall-.*')

	for root, dirs, _ in os.walk(rootdir):
		for folder in dirs:
			if regex.match(folder) and not folder.endswith('R'):
				sys.path.append(os.path.join(rootdir, folder, 'lib'))

	# Try to load the extension
	library_names = ['libpy_loaderd', 'libpy_loader']
	library_found = ''
	module = None

	# Find the library
	for name in library_names:
		try:
			module = __import__(name, globals(), locals())
			library_found = name
			break
		except ImportError as e:
			pass
		except:
			print("Unexpected error while loading the MetaCall Python Port", name, ":", sys.exc_info()[0])
			raise

	return module
