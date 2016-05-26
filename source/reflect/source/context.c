/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/context.h>
#include <reflect/scope.h>

typedef struct context_type
{
	char * name;
	scope sp;

} * context;

context context_create(char * name)
{
	if (name != NULL)
	{
		context ctx = malloc(sizeof(struct context_type));

		if (ctx != NULL)
		{
			ctx->name = name;
			ctx->sp = scope_create("global_namespace");

			return ctx;
		}
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

		free(ctx);
	}
}
