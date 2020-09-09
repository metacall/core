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
import json
import types

# Append environment variable or default install path when building manually (TODO: Cross-platform paths)
sys.path.append(os.environ.get('LOADER_LIBRARY_PATH', os.path.join(os.path.sep, 'usr', 'local', 'lib')));

# Find is MetaCall is installed as a distributable tarball (TODO: Cross-platform paths)
rootdir = os.path.join(os.path.sep, 'gnu', 'store');
regex = re.compile('.*-metacall-.*');

for root, dirs, _ in os.walk(rootdir):
	for folder in dirs:
		if regex.match(folder) and not folder.endswith('R'):
			sys.path.append(os.path.join(rootdir, folder, 'lib'));

# Try to load the extension
library_names = ['libpy_loaderd', 'py_loaderd', 'libpy_loader', 'py_loader'];
library_found = '';
module = None;

# Find the library
for name in library_names:
	try:
		module = __import__(name, globals(), locals());
		library_found = name;
		metacall_load_from_file = module.metacall_load_from_file;
		metacall_load_from_memory = module.metacall_load_from_memory;
		metacall = module.metacall;
		break
	except ImportError as e:
		pass
	except:
		print("Unexpected error while loading the MetaCall Python Port", name, ":", sys.exc_info()[0]);
		raise

# Check if library was found and print error message otherwhise
if library_found == '':
	print('\x1b[31m\x1b[1m', 'You do not have MetaCall installed or we cannot find it (', e, ')\x1b[0m');
	print('\x1b[33m\x1b[1m', 'If you do not have it installed, you have three options:', '\x1b[0m');
	print('\x1b[1m', '	1) Go to https://github.com/metacall/install and install it.', '\x1b[0m');
	print('\x1b[1m', '	2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.', '\033[0m');
	print('\x1b[1m', '	3) Be a x10 programmer and compile it by yourself, then define the install folder (if it is different from the default /usr/local/lib) in os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m');
	print('\x1b[33m\x1b[1m', 'If you have it installed in an non-standard folder, please define os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m');
	raise ImportError('MetaCall Python Port was not found');
else:
	print('MetaCall Python Port loaded:', library_found);

# TODO:

"""
# Wrap metacall inspect and transform the json string into a dict
def metacall_inspect():
	data = module.metacall_inspect()
	if data:
		dic = json.loads(data)
		try:
			del dic['__metacall_host__']
		except:
			pass
		return dic
	return dict()
"""


# Monkey patch

"""
# Override Import
def _import(name, *args, **kwargs):
	# TODO: Implement metacall import
	# Find in all folders if some file with the name exists
	if name == 'matplotlib':
		name = 'my_mocked_matplotlib'
	return _python_import(name, *args, **kwargs)

import builtins
_python_import = builtins.__import__
builtins.__import__ = _import
"""


# TODO:


"""
def setUpModule(nameMod):
    return sys.modules.setdefault(nameMod, types.ModuleType(nameMod))


def metacall_require(tag, namePath, id):
    metacall_load_from_file(tag, [namePath])
    inspectData = inspect()
    if not inspectData:
            print('\x1b[31m\x1b[1m',
                  'Inspect Data from metacall_inspect is empty\x1b[0m')
            return
    listOfScripts = inspectData[tag]
    script = next((x for x in listOfScripts if x.name == id), None)
    mod = setUpModule(id)
    for func in script['scope']['funcs']:
        mod.__dict__[func.name] = lambda *args: metacall(func.name, *args)
        pass
    pass


# Get Files from LOADER_SCRIPTS_PATH
scriptsFolder = None
try:
    scriptsFolder = os.environ['LOADER_SCRIPT_PATH']
except KeyError as e:
    print('\x1b[31m\x1b[1m',
          'There is no environment variable called LOADER_SCRIPT_PATH (', e, ')\x1b[0m')
    pass
if not os.path.isdir(scriptsFolder):
    print('\x1b[31m\x1b[1m',
          'LOADER_SCRIPTS_PATH should be a folder/directory (', e, ')\x1b[0m')
    pass

tags = {
    ['mock']: 'mock',
    ['js', 'node']: 'node',
    ['py']: 'py',
    ['rb']: 'rb',
    ['vb', 'cs']: 'cs'
}

files = os.listdir(scriptsFolder)
for script in files:
    scriptAbsPAth = os.path.join(scriptsFolder, script)
    if os.path.isfile(scriptAbsPAth):
        fileNameAndExtensionList = script.split('.')
        extension = fileNameAndExtensionList[1]
        mtag = tags[extension]
        metacall_require(
            mtag, scriptAbsPAth, fileNameAndExtensionList[0])
        pass
    pass
"""
