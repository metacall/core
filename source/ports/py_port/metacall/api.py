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
					return handle

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
		symbol_dict = dict(functools.reduce(lambda symbols, func: {**symbols, func['name']: lambda *args: metacall(func['name'], *args) }, handle['scope']['funcs'], {}))

		mod.__dict__.update(symbol_dict)

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
		'node': 'node',
		'js': 'node',
		# WebAssembly Loader
		'wat': 'wasm',
		# TypeScript Loader
		'ts': 'ts',
		'jsx': 'ts',
		'tsx': 'ts',
		# Note: By default js extension uses NodeJS loader instead of JavaScript V8
		# Probably in the future we can differenciate between them, but it is not trivial
	}

	package_extensions_to_tag = {
		# C# Loader
		'dll': 'cs',
		# WebAssembly Loader
		'wasm': 'wasm',
	}

	# Try to load it as a Python module
	mod = None

	with suppress(ImportError):
		mod = __python_import__(name, globals, locals, fromlist, level)

	if mod:
		return mod

	# Obtain the extension of the module if any
	extension = None if name.count('.') == 0 else name.split('.')[-1]

	# Load by extension if there is any (import puppeteer.js)
	if (
		extension in file_extensions_to_tag.keys()
		or extension in package_extensions_to_tag.keys()
	):
		# Get handle name without extension
		handle_name = name.split('.')[-2]

		# Check if it is already loaded in MetaCall
		handle = find_handle(name)

		if handle != None:
			# Generate the module from cached handle
			return generate_module(handle_name, handle)

		# This breaks a recursion loop when trying to load py files
		# The problem is basically that PyImport_Import in the loader tries to
		# load as module first, so it jumps to this method again. Probably
		# we should find a more efficient solution, for example reimplementing
		# PyImport_Import (which I failed) or checking if the port is present
		# and the import is monkey patched, so in this case we should populate
		# the real python __import__ from the port into the loader, so we can
		# avoid jumping always to the patched import, this is a workaround
		# that must be removed because it is dirty and slow (TODO)
		if extension == 'py':
			current_frame = inspect.currentframe()
			call_frame = inspect.getouterframes(current_frame, 2)
			if (
				call_frame[1][3] == 'metacall_load_from_file'
				or call_frame[1][3] == 'metacall_load_from_package'
				or call_frame[1][3] == 'metacall_load_from_memory'
			):
				return ImportException('MetaCall could not import:', name)

		if (
			extension in file_extensions_to_tag.keys()
			and metacall_load_from_file(
				file_extensions_to_tag[extension], [name]
			)
		) or (
			extension in package_extensions_to_tag.keys()
			and metacall_load_from_package(
				package_extensions_to_tag[extension], name
			)
		):
			handle = find_handle(name)
			if handle != None:
				# Generate the module from cached handle
				return generate_module(handle_name, handle)
	else:
		# Check if it is already loaded in MetaCall
		handle = find_handle(name)

		if handle != None:
			# Generate the module from cached handle
			return generate_module(name, handle)

		# Otherwhise, try to load it by guessing the loader
		tags = set(file_extensions_to_tag.values())

		# Remove mock and py in order to avoid mock to automatically load
		# or python to enter into an endless loop
		tags.discard('mock')
		tags.discard('py')

		for tag in list(tags):
			if metacall_load_from_file(tag, [name]):
				handle = find_handle(name)
				if handle != None:
					# Generate the module from cached handle
					return generate_module(name, handle)

	raise ImportException('MetaCall could not import:', name)

# Override Python import
builtins.__import__ = __metacall_import__
