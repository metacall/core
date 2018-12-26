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

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_id_size.h>

size_t value_type_id_size(type_id id)
{
	static const size_t type_id_size_list[TYPE_SIZE] =
	{
		sizeof(boolean),	/* TYPE_BOOL */
		sizeof(char),		/* TYPE_CHAR */
		sizeof(short),		/* TYPE_SHORT */
		sizeof(int),		/* TYPE_INT */
		sizeof(long),		/* TYPE_LONG */
		sizeof(float),		/* TYPE_FLOAT */
		sizeof(double),		/* TYPE_DOUBLE */
		sizeof(char *),		/* TYPE_STRING */
		sizeof(void *),		/* TYPE_BUFFER */
		sizeof(value *),	/* TYPE_ARRAY */
		sizeof(value *),	/* TYPE_MAP */
		sizeof(void *),		/* TYPE_PTR */
		(size_t)0			/* TYPE_NULL */
	};

	if (type_id_invalid(id) == 0)
	{
		return 0;
	}

	return type_id_size_list[id];
}
