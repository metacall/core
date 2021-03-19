/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading mock code at run-time into a process.
*
*/

#include <mock_loader/mock_loader_export.h>

#include <mock_loader/mock_loader_descriptor.h>
#include <mock_loader/mock_loader_interface.h>
#include <mock_loader/mock_loader_print.h>

loader_impl_export mock_loader_impl_export_singleton()
{
	static struct loader_impl_export_type mock_loader_impl_export = {
		&mock_loader_impl_descriptor_singleton,
		&mock_loader_impl_interface_singleton,
		&mock_loader_impl_print_singleton
	};

	return &mock_loader_impl_export;
}
