/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <py_loader/py_loader_impl.h>

#include <stdlib.h>

int py_loader_impl_initialize(loader_impl impl)
{
	return 1;
}

int py_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	return 1;
}

loader_handle py_loader_impl_load(loader_impl impl, loader_naming_name name)
{
	return NULL;
}

int py_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	return 1;
}

int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	return 1;
}

int py_loader_impl_destroy(loader_impl impl)
{
	return 1;
}
