/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <mock_loader/mock_loader_impl.h>
#include <mock_loader/mock_loader_interface.h>

#include <loader/loader_impl_interface.h>

loader_impl_interface mock_loader_impl_interface_singleton(void)
{
	static struct loader_impl_interface_type loader_impl_interface_mock = {
		&mock_loader_impl_initialize,
		&mock_loader_impl_execution_path,
		&mock_loader_impl_load,
		&mock_loader_impl_clear,
		&mock_loader_impl_register_types,
		&mock_loader_impl_discover,
		&mock_loader_impl_destroy
	};

	return &loader_impl_interface_mock;
}
