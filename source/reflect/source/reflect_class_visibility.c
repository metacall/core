/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_class_visibility.h>

#include <reflect/reflect_value_type.h>

struct visibility_string_length_type
{
	const char *str;
	size_t length;
};

#define CLASS_VISIBILITY_STRUCT(name, str, id) \
	{ \
		str, sizeof(str) - 1 \
	}

static struct visibility_string_length_type visibility_array[] = {
	CLASS_VISIBILITY_X(CLASS_VISIBILITY_STRUCT)
};

const char *class_visibility_string(enum class_visibility_id visibility)
{
	return visibility_array[visibility].str;
}

value class_visibility_value(enum class_visibility_id visibility)
{
	return value_create_string(visibility_array[visibility].str, visibility_array[visibility].length);
}

value class_visibility_value_pair(enum class_visibility_id visibility)
{
	static const char visibility_str[] = "visibility";
	value v = value_create_array(NULL, 2);
	value *v_array;

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);
	v_array[0] = value_create_string(visibility_str, sizeof(visibility_str) - 1);

	if (v_array[0] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	v_array[1] = class_visibility_value(visibility);

	if (v_array[1] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	return v;
}
