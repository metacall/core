#include <call/function.h>

typedef struct function_type
{
	char * name;
	argument_list signature;
	function_impl impl;
} * function;

function function_create(char * name, argument_list signature)
{
	if (name && signature && impl)
	{
		function func = malloc(sizeof(struct function_type));

		if (func)
		{
			func->name = name;
			func->signature = signature;
			//func->impl = ;

			return func;
		}
	}

	return NULL;
}

char * function_get_name(function func)
{
	return func->name;
}

argument_list function_get_signature(function func)
{
	return func->signature;
}

function_impl function_get_impl(function func)
{
	return func->impl;
}


void function_push(function func, int index, void * data)
{
	if (func && index >= 0 && index < argument_list_count(func->signature) && data)
	{
		
	}
}

int function_push_va(function func, va_list va)
{
	// push a list of arguments
}

void function_call_impl(function func)
{
	// execute the call
}

void function_call_va(function func, va_list va)
{
	if (function_push_va(func, va) == 0)
	{
		function_call_impl(func);
	}
}

void function_call(function func, ...)
{
	va_list va;

	va_start(va, func);

	function_call_va(func, va);

	va_end(va);
}

void function_destroy(function func)
{
	if (func)
	{
		// context_detach(func->context, func);

		argument_list_destroy(func->signature);

		free(func);
	}
}
