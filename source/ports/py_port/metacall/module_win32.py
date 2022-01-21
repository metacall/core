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
import ctypes
from ctypes import wintypes, windll

def metacall_module_load():
	def enum_process_modules():
		# Get handle of current process
		kernel32 = windll.kernel32
		kernel32.GetCurrentProcess.restype = ctypes.c_void_p
		hProcess = kernel32.GetCurrentProcess()

		# Load EnumProcessModules either from kernel32.dll or psapi.dll
		try:
			EnumProcessModulesProc = windll.psapi.EnumProcessModules
		except AttributeError:
			EnumProcessModulesProc = windll.kernel32.EnumProcessModules
		EnumProcessModulesProc.restype = ctypes.c_bool
		EnumProcessModulesProc.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p), ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong)]

		hProcess = kernel32.GetCurrentProcess()
		hMods = (ctypes.c_void_p * 1024)()
		cbNeeded = ctypes.c_ulong()
		if EnumProcessModulesProc(hProcess, hMods, ctypes.sizeof(hMods), ctypes.byref(cbNeeded)):
			return hMods
		return None

	def get_loaded_module(modules, module_name):
		kernel32 = windll.kernel32
		kernel32.GetModuleFileNameA.restype = ctypes.c_ulong
		kernel32.GetModuleFileNameA.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_ulong]

		if modules is None:
			return None
		for module in modules:
			cPath = ctypes.c_char_p(b'\0' * 1024)
			kernel32.GetModuleFileNameA(module, cPath, ctypes.c_ulong(1024))
			path = cPath.value
			if path.endswith(bytes(module_name, 'ascii')):
				return module
		return None

	# Retrieve all loaded modules in the process
	modules = enum_process_modules()

	# Get environment variable or default install path when building manually
	base_path = os.environ.get('LOADER_LIBRARY_PATH', os.path.join(os.path.sep, 'C:', 'Program Files', 'MetaCall'))
	library_names = ['py_loaderd', 'py_loader']

	for name in library_names:
		runtime_module_handle = get_loaded_module(modules, os.path.join(os.path.sep, base_path, name + '.dll'))
		runtime_module = ctypes.CDLL('', handle = runtime_module_handle) # cdecl calling convention

		if runtime_module != None:
			func_name = 'PyInit_' + name
			if runtime_module[func_name]:
				init = runtime_module[func_name]
				init.restype = ctypes.py_object
				init.argtypes = None
				return init()

	return None
