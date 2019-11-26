#!/usr/bin/env python3

global_var = 0

def __metacall_initialize__() -> int:
	global global_var
	global_var = 15
	print('PYTHON INITIALIZE HOOK');
	return 0;

def intermediate_function() -> int:
	print('intermediate function');
	return global_var;

def __metacall_finalize__() -> int:
	print('PYTHON FINALIZE HOOK');
	return 0;
