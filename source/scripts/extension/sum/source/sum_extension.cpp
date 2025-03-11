/*
 *	Extension Library by Parra Studios
 *	An extension for sum numbers.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.h>
#include <sum_extension/sum_extension.h>

#include <plugin/plugin_interface.hpp>

void *sum(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to sum", METACALL_LONG, METACALL_LONG);

	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);
	long result = left + right;

	printf("%ld + %ld = %ld\n", left, right, result);

	return metacall_value_create_long(result);
}

int sum_extension(void *loader, void *handle)
{
	/* Register function */
	EXTENSION_FUNCTION(METACALL_LONG, sum, METACALL_LONG, METACALL_LONG);

	return 0;
}
