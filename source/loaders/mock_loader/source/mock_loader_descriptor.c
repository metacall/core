/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A plugin for loading mock code at run-time into a process.
*
*/

#include <mock_loader/mock_loader_descriptor.h>

loader_descriptor mock_loader_impl_descriptor_instance(void);

loader_impl_descriptor mock_loader_impl_descriptor_singleton(void)
{
	static struct loader_impl_descriptor_type loader_impl_descriptor_mock = {
		&mock_loader_impl_descriptor_instance
	};

	return &loader_impl_descriptor_mock;
}

loader_descriptor mock_loader_impl_descriptor_instance(void)
{
	/* Copyright */
	static struct loader_descriptor_copyright_type loader_descriptor_copyright_mock = {
		"Vicente Eduardo Ferrer Garcia <vic798@gmail.com>",
		"Vicente Eduardo Ferrer Garcia <vic798@gmail.com>",
		"http://parrastudios.com",
		{ 11, "June", 2016 },
		"Parra Studios",
		"Vicente Eduardo Ferrer Garcia <vic798@gmail.com>",
		{ 0x00000001,
			"0.0.1" },
		"Propietary"
	};

	/* Language */
	static struct loader_descriptor_language_type loader_descriptor_language_mock = {
		"Mocked Language",
		{ 0x00000000,
			"0.0.0" }
	};

	/* Default extensions */
	static const char *mock_default_extensions[] = {
		"mock"
	};

	/* Technology */
	static struct loader_descriptor_technology_type loader_descriptor_technology_mock = {
		"Mock Technology",
		"Mocked Vendor",
		"http://mock",
		{ 0x00000000,
			"0.0.0" },
		{ mock_default_extensions,
			(uint32_t)(sizeof(mock_default_extensions) / sizeof(mock_default_extensions[0])) }
	};

	/* Platform */
	static struct loader_descriptor_platform_type loader_descriptor_platform_mock = {
		/* TODO */

		"todo-architecture",
		"todo-compiler",
		"todo-os",
		"todo-os-family"
	};

	/* Main loader information */
	static struct loader_descriptor_type loader_descriptor_mock = {
		"Mock Loader Plugin",
		"mock",
		"Mock loader in order to test MetaCall core functionalities without "
		"need of having a run-time environment installed.",
		&loader_descriptor_copyright_mock,
		&loader_descriptor_language_mock,
		&loader_descriptor_technology_mock,
		&loader_descriptor_platform_mock
	};

	return &loader_descriptor_mock;
}
