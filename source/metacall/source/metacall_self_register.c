/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/*
 * Hand-written prototype of metacall self-registration. This file will be
 * replaced by build-time generated bindings once the bindgen tool exists.
 * For now it registers a single API (metacall_print_info) to validate the
 * end-to-end approach: scripts in any loaded language can call metacall_*
 * APIs without going through the C loader.
 */

#include <metacall/metacall.h>
#include <metacall/metacall_self_register.h>

#include <string.h>

static void *metacall_self_register_print_info(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	const char *info = metacall_print_info();

	return metacall_value_create_string(info, info == NULL ? 0 : strlen(info));
}

int metacall_self_register_all(void)
{
	if (metacall_register("metacall_print_info", &metacall_self_register_print_info, NULL, METACALL_STRING, 0) != 0)
	{
		return 1;
	}

	return 0;
}
