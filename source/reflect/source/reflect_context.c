/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>

#include <stdlib.h>
#include <string.h>

typedef struct context_type
{
	char * name;
	scope sp;

} * context;

context context_create(const char * name)
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

const char * context_name(context ctx)
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
