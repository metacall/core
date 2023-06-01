#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
import json
import inspect # TODO: Remove this, check the monkey patching

if sys.platform == 'win32':
	from metacall.module_win32 import metacall_module_load
elif sys.platform == 'linux':
	from metacall.module_linux import metacall_module_load
elif sys.platform == 'darwin' or sys.platform == 'cygwin':
	print('\x1b[31m\x1b[1m', 'The platform', sys.platform, 'has not been not tested, but we are using linux module as a fallback.', '\x1b[0m')
	# TODO: Probably it won't work, but we use it as a fallback, implement other platforms
	from metacall.module_linux import metacall_module_load
else:
	raise ImportError('MetaCall Python Port is not implemented under this platform.')

# Load metacall extension depending on the platform
module = metacall_module_load()

# Check if library was found and print error message otherwhise
if module == None:
	print('\x1b[31m\x1b[1m', 'You do not have MetaCall installed or we cannot find it.', '\x1b[0m')
	print('\x1b[1m', 'Looking for it in the following paths:', sys.path, '\x1b[0m')
	print('\x1b[33m\x1b[1m', 'If you do not have it installed, you have three options:', '\x1b[0m')
	print('\x1b[1m', '	1) Go to https://github.com/metacall/install and install it.', '\x1b[0m')
	print('\x1b[1m', '	2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.', '\033[0m')
	print('\x1b[1m', '	3) Be a x10 programmer and compile it by yourself, then define the install folder (if it is different from the default /usr/local/lib) in os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m')
	print('\x1b[33m\x1b[1m', 'If you have it installed in an non-standard folder, please define os.environ[\'LOADER_LIBRARY_PATH\'].', '\x1b[0m')
	raise ImportError('MetaCall Python Port was not found')

# Load from file
def metacall_load_from_file(tag, paths):
	return module.metacall_load_from_file(tag, paths)

# Load from package
def metacall_load_from_package(tag, path):
	return module.metacall_load_from_package(tag, path)

# Load from memory
def metacall_load_from_memory(tag, buffer):
	return module.metacall_load_from_memory(tag, buffer)

# Invocation
def metacall(function_name, *args):
	return module.metacall(function_name, *args)

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

# Monkey patching
import builtins
import types
from contextlib import suppress
import functools

# Save the original Python import
__python_import__ = builtins.__import__

# Define proper import exception depending on Python support
ImportException = ImportError

try:
	try:
		raise ModuleNotFoundError()
	except ModuleNotFoundError as e:
		ImportException = ModuleNotFoundError
except NameError as e:
	ImportException = ImportError

def __metacall_import__(name, globals=None, locals=None, fromlist=(), level=0):

	def find_handle(handle_name):
		metadata = metacall_inspect()

		for loader in metadata.keys():
			for handle in metadata[loader]:
				if handle['name'] == handle_name:
					return dict(functools.reduce(lambda symbols, func: {**symbols, func['name']: lambda *args: metacall(func['name'], *args) }, handle['scope']['funcs'], {}))

		return None

	def generate_module(handle_name, handle):
		mod = sys.modules.setdefault(handle_name, types.ModuleType(handle_name))

		# Set a few properties required by PEP 302
		base_path = os.environ.get('LOADER_SCRIPT_PATH', os.getcwd())
		mod.__file__ = os.path.join(base_path, handle_name)
		mod.__name__ = handle_name
		mod.__path__ = base_path
		# TODO: Using os.__loader__ instead of self until we implement the custom loader class
		mod.__loader__ = os.__loader__ # self
		# PEP-366 specifies that package's set __package__ to
		# their name, and modules have it set to their parent package (if any)
		# TODO (https://pymotw.com/3/sys/imports.html):
		# if self.is_package(handle_name):
		# 	mod.__package__ = handle_name
		# else:
		# 	mod.__package__ = '.'.join(handle_name.split('.')[:-1])
		mod.__package__ = handle_name

		# Add the symbols to the module
		mod.__dict__.update(handle)

		return mod

	# Map file extension to tags
	file_extensions_to_tag = {
		# Mock Loader
		'mock': 'mock',
		# Python Loader
		'py': 'py',
		# Ruby Loader
		'rb': 'rb',
		# C# Loader
		'cs': 'cs',
		'vb': 'cs',
		# Cobol Loader
		'cob': 'cob',
		'cbl': 'cob',
		'cpy': 'cob',
		# NodeJS Loader
		'node': 'node', # TODO: Load by package on node is not implemented or it is unnecesary
		'js': 'node',
		# WebAssembly Loader
		'wat': 'wasm',
		# TypeScript Loader
		'ts': 'ts',
		'jsx': 'ts',
		'tsx': 'ts',
		# Rust Loader
		'rs': 'rs',
		# Note: By default js extension uses NodeJS loader instead of JavaScript V8
		# Probably in the future we can differenciate between them, but it is not trivial
	}

	package_extensions_to_tag = {
		# C# Loader
		'dll': 'cs',
		# WebAssembly Loader
		'wasm': 'wasm',
		# Rust Loader
		'rlib': 'rs',
	}

	# Set containing all tags
	available_tags = set({**file_extensions_to_tag, **package_extensions_to_tag})

	# Try to load by module: import metacall.node.ramda
	metacall_module_import = name.split('.')
	metacall_module_import_size = len(metacall_module_import)

	if metacall_module_import_size > 1 and metacall_module_import[0] == 'metacall':
		if not metacall_module_import[1] in available_tags:
			raise ImportException(f'MetaCall could not import: {name}; the loader {metacall_module_import[1]} does not exist')

		def _metacall_module_import(module_tag, module_name, return_root):
			# Check if it is already loaded
			metacall_module = sys.modules['metacall']
			if hasattr(metacall_module, module_tag):
				metacall_module_tag = getattr(metacall_module, module_tag)
				if hasattr(metacall_module_tag, module_name):
					if return_root == True:
						return getattr(metacall_module_tag, module_name)
					else:
						return metacall_module_tag

			handle = module.metacall_load_from_file_export(
				module_tag, [module_name]
			)

			if handle != None:
				# Generate the module
				imported_module = generate_module(module_name, handle)

				if imported_module is None:
					raise ImportException(f'MetaCall could not import: {name}; failed to generate the module')

				# Add the new functions to metacall module
				metacall_module = sys.modules['metacall']
				if not hasattr(metacall_module, module_tag):
					setattr(metacall_module, module_tag, types.ModuleType(module_tag))

				metacall_module_tag = getattr(metacall_module, module_tag)
				setattr(metacall_module_tag, module_name, imported_module)
				setattr(metacall_module, module_tag, metacall_module_tag)

				if return_root == True:
					return metacall_module
				else:
					return metacall_module_tag
			else:
				raise ImportException(f'MetaCall could not import: {name}')

		# Case 1: import metacall.node.ramda && Case 2: from metacall.node.ramda import func1, func2, func3
		if metacall_module_import_size >= 3:
			module_tag = metacall_module_import[1]
			module_name = '.'.join(metacall_module_import[2:])
			return _metacall_module_import(module_tag, module_name, True)

		# Case 3: from metacall.node import ramda, express
		elif metacall_module_import_size == 2:
			if fromlist is None:
				raise ImportException(f'MetaCall could not import: {name}; you need to specify the submodules: from {name} import moduleA, moduleB')

			metacall_module_tag = None
			module_tag = metacall_module_import[1]

			for module_name in fromlist:
				metacall_module_tag = _metacall_module_import(module_tag, module_name, False)

			return metacall_module_tag

	# Try to load by extension: import script.js
	filename, extension = os.path.splitext(name)

	if extension != '':
		ext = extension[1:]

		if (
			ext in file_extensions_to_tag.keys()
			or ext in package_extensions_to_tag.keys()
		):
			# Check if it is already loaded in MetaCall
			handle = find_handle(name)

			if handle != None:
				# Generate the module from cached handle
				return generate_module(filename, handle)

			if ext in file_extensions_to_tag.keys():
				# Load from file
				file_tag = file_extensions_to_tag[ext]

				if file_tag == 'py':
					return __python_import__(filename, globals, locals, fromlist, level)
				else:
					handle = module.metacall_load_from_file_export(file_tag, [name])

			elif ext in package_extensions_to_tag.keys():
				# Load from package
				package_tag = package_extensions_to_tag[ext]

				if package_tag == 'py':
					return __python_import__(filename, globals, locals, fromlist, level)
				else:
					handle = module.metacall_load_from_package_export(package_tag, name)

			if handle != None:
				# Generate the module
				return generate_module(filename, handle)

	# Load with Python
	return __python_import__(name, globals, locals, fromlist, level)

# Override Python import
builtins.__import__ = __metacall_import__
