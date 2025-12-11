#!/usr/bin/env python3

#	MetaCall Python Port by Parra Studios
#	A frontend for Python language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
import re
import sys
import json
import ctypes

def find_files_recursively(root_dir, pattern):
	regex = re.compile(pattern)
	matches = []
	for dirpath, dirnames, filenames in os.walk(root_dir):
		for filename in filenames:
			if regex.search(filename):
				matches.append(os.path.join(dirpath, filename))
	return matches

def platform_install_paths():
	if sys.platform == 'win32':
		return {
			'paths': [ os.path.join(os.environ.get('LOCALAPPDATA', ''), 'MetaCall', 'metacall') ],
			'name': r'^metacall(d)?\.dll$'
		}
	elif sys.platform == 'darwin':
		return {
			'paths': [ '/opt/homebrew/lib/', '/usr/local/lib/' ],
			'name': r'^libmetacall(d)?\.dylib$'
		}
	elif sys.platform == 'linux':
		return {
			'paths': [ '/usr/local/lib/', '/gnu/lib/' ],
			'name': r'^libmetacall(d)?\.so$'
		}
	else:
		raise RuntimeError(f"Platform {sys.platform} not supported")

def search_paths():
	custom_path = os.environ.get('METACALL_INSTALL_PATH')
	if custom_path:
		return {
			'paths': [ custom_path ],
			'name': r'^(lib)?metacall(d)?\.(so|dylib|dll)$'
		}

	return platform_install_paths()

def find_library():
	search_data = search_paths()

	for path in search_data['paths']:
		files = find_files_recursively(path, search_data['name'])
		if files:
			return files[0]

	raise ImportError("""
		MetaCall library not found, if you have it in a special folder, define it through METACALL_INSTALL_PATH'.
		"""
		+ "Looking for it in the following paths: " + ', '.join(search_data['paths']) + """
		If you do not have it installed, you have three options:
			1) Go to https://github.com/metacall/install and install it.
			2) Contribute to https://github.com/metacall/distributable by providing support for your platform and architecture.
			3) Be a x10 programmer and compile it by yourself, then define the install folder if it is different from the default in os.environ['METACALL_INSTALL_PATH'].
	""")

def metacall_module_load():
	# Check if it is loaded from MetaCall or from Python
	if 'py_port_impl_module' in sys.modules:
		return sys.modules['py_port_impl_module']

	# Define the Python host
	os.environ['METACALL_HOST'] = 'py'

	# Find the shared library
	library_path = find_library()

	# Load MetaCall
	lib = ctypes.CDLL(library_path, mode=ctypes.RTLD_GLOBAL)

	# Python Port must have been loaded at this point
	if 'py_port_impl_module' in sys.modules:
		port = sys.modules['py_port_impl_module']

		# For some reason, Windows deadlocks on initializing asyncio
		# but if it is delayed, it works, so we initialize it after here
		if sys.platform == 'win32':
			port.py_loader_port_asyncio_initialize()

		return port
	else:
		raise ImportError(
			'MetaCall was found but failed to load'
		)

# Load metacall extension depending on the platform
module = metacall_module_load()

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

# Await invocation - returns a Future
def metacall_await(function_name, *args):
	"""
	Call an async function from another runtime and return a Future.

	The returned Future can be awaited in async context:
		result = await metacall_await('async_func', arg1, arg2)

	Args:
		function_name: Name of the async function to call
		*args: Arguments to pass to the function

	Returns:
		A Future that resolves to the function's return value
	"""
	return module.metacall_await(function_name, *args)


class MetaCallFunction:
	"""
	Wrapper for MetaCall functions that supports both sync and async calls.

	For synchronous functions:
		func = MetaCallFunction('my_sync_func', is_async=False)
		result = func(arg1, arg2)

	For asynchronous functions:
		func = MetaCallFunction('my_async_func', is_async=True)
		result = await func(arg1, arg2)

	The wrapper can also be called with explicit methods:
		result = func.call(arg1, arg2)           # Sync call
		result = await func.async_call(arg1, arg2)  # Async call
	"""

	def __init__(self, name, is_async=False):
		"""
		Initialize a MetaCallFunction wrapper.

		Args:
			name: The name of the function in MetaCall
			is_async: Whether the function is asynchronous
		"""
		self._name = name
		self._is_async = is_async
		self.__name__ = name
		self.__qualname__ = name
		self.__doc__ = f"MetaCall function '{name}' ({'async' if is_async else 'sync'})"

	@property
	def name(self):
		"""Get the function name."""
		return self._name

	@property
	def is_async(self):
		"""Check if the function is asynchronous."""
		return self._is_async

	def __call__(self, *args):
		"""
		Call the function.

		For async functions, returns a coroutine that must be awaited.
		For sync functions, returns the result directly.
		"""
		if self._is_async:
			return self._async_call_impl(*args)
		else:
			return metacall(self._name, *args)

	def call(self, *args):
		"""
		Synchronous call to the function.

		For async functions, this will block until the result is available.
		"""
		if self._is_async:
			import asyncio
			future = module.metacall_await(self._name, *args)
			# If we're in an event loop, wrap the future
			try:
				loop = asyncio.get_running_loop()
				# We're in an async context, return awaitable
				return asyncio.wrap_future(future, loop=loop)
			except RuntimeError:
				# No running loop, block on the future
				return future.result()
		else:
			return metacall(self._name, *args)

	async def _async_call_impl(self, *args):
		"""Internal async implementation."""
		import asyncio
		future = module.metacall_await(self._name, *args)
		return await asyncio.wrap_future(future)

	async def async_call(self, *args):
		"""
		Asynchronous call to the function.

		Can be used for both sync and async functions.
		For sync functions, the call is wrapped in an executor.
		"""
		if self._is_async:
			return await self._async_call_impl(*args)
		else:
			import asyncio
			loop = asyncio.get_running_loop()
			return await loop.run_in_executor(None, lambda: metacall(self._name, *args))

	def __repr__(self):
		return f"MetaCallFunction('{self._name}', is_async={self._is_async})"

	def __str__(self):
		return f"<metacall function '{self._name}' ({'async' if self._is_async else 'sync'})>"


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

# Value API for handling pointers
def metacall_value_create_ptr(ptr):
	return module.metacall_value_create_ptr(ptr)

# Value API for getting the pointer to a value
def metacall_value_reference(v):
	return module.metacall_value_reference(v)

# Value API for getting the value of a pointer
def metacall_value_dereference(ptr):
	return module.metacall_value_dereference(ptr)

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
		"""Find handle and create wrappers with async detection."""
		metadata = metacall_inspect()

		for loader in metadata.keys():
			for handle in metadata[loader]:
				if handle['name'] == handle_name:
					symbols = {}
					for func in handle['scope']['funcs']:
						func_name = func['name']
						# Check for async property in function metadata
						is_async = func.get('async', False)
						symbols[func_name] = MetaCallFunction(func_name, is_async=is_async)
					return symbols

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
		# C Loader
		'c': 'c'

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
