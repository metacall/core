/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <adt/adt_comparable.h>

#include <string.h>

int comparable_callback_str(const comparable a, const comparable b)
{
	const char *str_a = a;
	const char *str_b = b;

	return strcmp(str_a, str_b);
}

int comparable_callback_int(const comparable a, const comparable b)
{
	const int *int_ptr_a = a;
	const int *int_ptr_b = b;

	const int int_var_a = *int_ptr_a;
	const int int_var_b = *int_ptr_b;

	if (int_var_a < int_var_b)
	{
		return -1;
	}
	else if (int_var_a > int_var_b)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int comparable_callback_ptr(const comparable a, const comparable b)
{
	const void *void_ptr_a = a;
	const void *void_ptr_b = b;

	if (void_ptr_a < void_ptr_b)
	{
		return -1;
	}
	else if (void_ptr_a > void_ptr_b)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
