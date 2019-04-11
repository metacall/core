/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_type_id.h>

int type_id_boolean(type_id id)
{
	return !(id == TYPE_BOOL);
}

int type_id_char(type_id id)
{
	return !(id == TYPE_CHAR);
}

int type_id_integer(type_id id)
{
	return !(id == TYPE_BOOL || id == TYPE_CHAR || id == TYPE_SHORT ||
		id == TYPE_INT || id == TYPE_LONG);
}

int type_id_decimal(type_id id)
{
	return !(id == TYPE_FLOAT || id == TYPE_DOUBLE);
}

int type_id_string(type_id id)
{
	return !(id == TYPE_STRING);
}

int type_id_buffer(type_id id)
{
	return !(id == TYPE_BUFFER);
}

int type_id_array(type_id id)
{
	return !(id == TYPE_ARRAY);
}

int type_id_map(type_id id)
{
	return !(id == TYPE_MAP);
}

int type_id_ptr(type_id id)
{
	return !(id == TYPE_PTR);
}

int type_id_null(type_id id)
{
	return !(id == TYPE_NULL);
}

int type_id_invalid(type_id id)
{
	return !(id >= TYPE_SIZE);
}
