/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <funchook_detour/funchook_detour.h>
#include <funchook_detour/funchook_detour_impl.h>

/* -- Methods -- */

detour_interface funchook_detour_impl_interface_singleton()
{
	static struct detour_interface_type interface_instance_funchook =
	{
		&funchook_detour_impl_initialize,
		&funchook_detour_impl_install,
		&funchook_detour_impl_uninstall,
		&funchook_detour_impl_destroy
	};

	return &interface_instance_funchook;
}

const char * funchook_detour_print_info()
{
	static const char funchook_detour_info[] =
		"FuncHook Detour Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef FUNCHOOK_DETOUR_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return funchook_detour_info;
}
