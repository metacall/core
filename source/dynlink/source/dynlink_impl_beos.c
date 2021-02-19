/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */
/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <be/kernel/image.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_beos(void)
{
	static const char extension_beos[0x03] = "so";

	return extension_beos;
}

void dynlink_impl_interface_get_name_beos(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, "lib", length);

	strncat(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
}

dynlink_impl dynlink_impl_interface_load_beos(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	int flags_impl;

	image_id impl;

	DYNLINK_FLAGS_SET(flags_impl, 0);

	impl = load_add_on(dynlink_get_name_impl(handle));

	if (impl < B_NO_ERROR)
	{
		return (dynlink_impl)impl;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: failed to load BeOS/Haiku image add-on");

	return NULL;
}

int dynlink_impl_interface_symbol_beos(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	void * symbol = NULL;

	int err = get_image_symbol((image_id)impl, name, B_SYMBOL_TYPE_ANY, &symbol);

	(void)handle;

	if (err != B_OK)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: failed to load BeOS/Haiku symbol %s", name);
		return 1;
	}

	*addr = (dynlink_symbol_addr)symbol;

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_beos(dynlink handle, dynlink_impl impl)
{
	(void)handle;

	#if defined(__ADDRESS_SANITIZER__)
		/* Disable dlclose when running with address sanitizer in order to maintain stacktraces */
		(void)impl;
		return 0;
	#else
		return ((image_id)impl > 0) && (unload_add_on((image_id)impl) < B_NO_ERROR);
	#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton_beos(void)
{
	static struct dynlink_impl_interface_type impl_interface_beos =
	{
		&dynlink_impl_interface_extension_beos,
		&dynlink_impl_interface_get_name_beos,
		&dynlink_impl_interface_load_beos,
		&dynlink_impl_interface_symbol_beos,
		&dynlink_impl_interface_unload_beos
	};

	return &impl_interface_beos;
}
