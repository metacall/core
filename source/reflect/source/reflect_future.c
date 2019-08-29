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

#include <reflect/reflect_future.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct future_type
{
	future_impl impl;
	future_interface interface;
};

future future_create(future_impl impl, future_impl_interface_singleton singleton)
{
	future f = malloc(sizeof(struct future_type));

	if (f == NULL)
	{
		return NULL;
	}

	f->impl = impl;

	if (singleton)
	{
		f->interface = singleton();
	}
	else
	{
		f->interface = NULL;
	}

	if (f->interface != NULL && f->interface->create != NULL)
	{
		if (f->interface->create(f, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid future create callback <%p>", f->interface->create);

			free(f);

			return NULL;
		}
	}

	return f;
}

void future_destroy(future f)
{
	if (f != NULL)
	{
		if (f->interface != NULL && f->interface->destroy != NULL)
		{
			f->interface->destroy(f, f->impl);
		}

		free(f);
	}
}
