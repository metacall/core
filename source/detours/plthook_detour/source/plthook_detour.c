/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <plthook_detour/plthook_detour.h>
#include <plthook_detour/plthook_detour_impl.h>

/* -- Methods -- */

detour_interface plthook_detour_impl_interface_singleton(void)
{
	static struct detour_interface_type interface_instance_plthook = {
		&plthook_detour_impl_initialize_file,
		&plthook_detour_impl_initialize_handle,
		&plthook_detour_impl_initialize_address,
		&plthook_detour_impl_enumerate,
		&plthook_detour_impl_replace,
		&plthook_detour_impl_error,
		&plthook_detour_impl_destroy
	};

	return &interface_instance_plthook;
}

const char *plthook_detour_print_info(void)
{
	static const char plthook_detour_info[] =
		"PLTHook Detour Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef PLTHOOK_DETOUR_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return plthook_detour_info;
}
