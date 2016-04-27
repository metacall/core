#include <call/function.h>

typedef struct function_type
{
	char * name;
	argument_list signature;
	function_impl impl;
} * function;

function function_create(char * name, argument_list signature)
{
	if (name != NULL && signature != NULL)
	{
		function func = malloc(sizeof(struct function_type));

		if (func != NULL)
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
	if (func != NULL)
	{
		return func->name;
	}

	return NULL;
}

argument_list function_get_signature(function func)
{
	if (func != NULL)
	{
		return func->signature;
	}

	return NULL;
}

function_impl function_get_impl(function func)
{
	if (func != NULL)
	{
		return func->impl;
	}

	return NULL;
}

void function_push(function func, int index, void * data)
{
	if (func != NULL && index >= 0 && index < argument_list_count(func->signature) && data != NULL)
	{
		
	}
}

int function_push_va(function func, va_list va)
{
	// push a list of arguments
}

void function_call(function func, va_list va)
{
	/*
	va_list va;

	va_start(va, func);

	function_call_va(func, va);

	va_end(va);
	*/

	if (function_push_va(func, va) == 0)
	{
		// function_call_impl(func);
	}
}

void function_destroy(function func)
{
	if (func != NULL)
	{
		// context_detach(func->context, func);

		argument_list_destroy(func->signature);

		free(func);
	}
}
