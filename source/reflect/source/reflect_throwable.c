/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_throwable.h>

#include <reflect/reflect_value_type.h>

#include <stdlib.h>

struct throwable_type
{
	value v;
};

throwable throwable_create(value v)
{
	throwable th = malloc(sizeof(struct throwable_type));

	if (th == NULL)
	{
		return NULL;
	}

	if (v == NULL)
	{
		/* In case of NULL, create a default throwable with a null value */
		th->v = value_create_null();
	}
	else if (value_type_id(v) == TYPE_THROWABLE)
	{
		/* This will flatten the type so it does not enter into a recursive state */
		throwable inner = value_to_throwable(v);

		/* Copy the the value from the throwable passed in the constructor */
		th->v = value_type_copy(inner->v);
	}
	else
	{
		/* Otherwise just set the value (it is supposed to be copied or allocated from outside of this constructor) */
		th->v = v;
	}

	if (th->v == NULL)
	{
		free(th);
		return NULL;
	}

	return th;
}

value throwable_value(throwable th)
{
	if (th == NULL)
	{
		return NULL;
	}

	return th->v;
}

void throwable_destroy(throwable th)
{
	if (th != NULL)
	{
		value_type_destroy(th->v);
		free(th);
	}
}
