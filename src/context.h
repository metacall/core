#ifndef CONTEXT_H
#define CONTEXT_H 1

#include <call/function.h>

#ifdef __cplusplus
extern "C" {
#endif

enum context_language_id
{
	LANGUAGE_JAVASCRIPT,
	LANGUAGE_PYTHON,
	LANGUAGE_RUBY,

	LANGUAGE_COUNT
};

typedef struct context_type * context;

context context_create(char * name, enum context_language_id lang);

void context_add_function(function func);

//void context_add_*(...);

function context_get_function(char * name);

//... context_get_*(char * name);

void context_destroy(context ctx);

#ifdef __cplusplus
}
#endif

#endif // CONTEXT_H
