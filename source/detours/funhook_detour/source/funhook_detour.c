/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <funhook_detour/funhook_detour.h>
#include <funhook_detour/funhook_detour_impl.h>

/* -- Methods -- */

detour_interface funhook_detour_impl_interface_singleton()
{
	static struct detour_interface_type interface_instance_funhook =
	{
		&funhook_detour_impl_initialize,
		&funhook_detour_impl_install,
		&funhook_detour_impl_trampoline,
		&funhook_detour_impl_uninstall,
		&funhook_detour_impl_destroy
	};

	return &interface_instance_funhook;
}

const char * funhook_detour_print_info()
{
	static const char funhook_detour_info[] =
		"FuncHook Detour Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef FUNHOOK_DETOUR_STATIC_DEFINE
			"Compiled as static library type\n"
		#else
			"Compiled as shared library type\n"
		#endif

		"\n";

	return funhook_detour_info;
}
