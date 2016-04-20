#include <call/context.h>

#include <string.h>

#define CONTEXT_NAME_SIZE 0xFF

typedef struct context_type
{
	char name[CONTEXT_NAME_SIZE];
	enum context language_id lang;
	//hashmap function_map;
} * contoext;

context context_create(char * name, enum context_language_id lang)
{
	if (name && lang < LANGUAGE_COUNT)
	{
		context ctx = malloc(sizeof(struct context_type));

		if (ctx)
		{
			ctx->name = name;
			ctx->lang = lang;

			return ctx;
		}
	}

	return NULL;
}

void context_add_function(function func);

//void context_add_*(...);

function context_get_function(char * name);

//... context_get_*(char * name);

void context_destroy(context ctx);

#endif // CONTEXT_H
