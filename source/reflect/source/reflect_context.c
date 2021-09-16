/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>

#include <stdlib.h>
#include <string.h>

struct context_type
{
	char *name;
	scope sp;
};

context context_create(const char *name)
{
	if (name != NULL)
	{
		context ctx = malloc(sizeof(struct context_type));

		if (ctx != NULL)
		{
			size_t name_size = strlen(name) + 1;

			ctx->name = malloc(sizeof(char) * name_size);

			if (ctx->name == NULL)
			{
				/* error */

				free(ctx);

				return NULL;
			}

			memcpy(ctx->name, name, name_size);

			ctx->sp = scope_create("global_namespace");

			return ctx;
		}
	}

	return NULL;
}

const char *context_name(context ctx)
{
	if (ctx != NULL)
	{
		return ctx->name;
	}

	return NULL;
}

scope context_scope(context ctx)
{
	if (ctx != NULL)
	{
		return ctx->sp;
	}

	return NULL;
}

int context_append(context dest, context src)
{
	return scope_append(dest->sp, src->sp);
}

int context_contains(context dest, context src, char **duplicated)
{
	return scope_contains(dest->sp, src->sp, duplicated);
}

int context_remove(context dest, context src)
{
	return scope_remove(dest->sp, src->sp);
}

void context_destroy(context ctx)
{
	if (ctx != NULL)
	{
		scope_destroy(ctx->sp);

		if (ctx->name != NULL)
		{
			free(ctx->name);
		}

		free(ctx);
	}
}
