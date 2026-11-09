/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_accessor.h>

#include <stddef.h>

const char *accessor_key(struct accessor_type *accessor)
{
	if (accessor == NULL)
	{
		return NULL;
	}

	return accessor->data.key;
}

constructor accessor_constructor(struct accessor_type *accessor)
{
	if (accessor == NULL)
	{
		return NULL;
	}

	return accessor->data.ctor;
}

attribute accessor_attribute(struct accessor_type *accessor)
{
	if (accessor == NULL)
	{
		return NULL;
	}

	return accessor->data.attr;
}

method accessor_method(struct accessor_type *accessor)
{
	if (accessor == NULL)
	{
		return NULL;
	}

	return accessor->data.m;
}
