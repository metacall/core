#ifndef FUNCTION_H
#define FUNCTION_H 1

#include <call/argument.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#define FUNCTION_ARGS_SIZE 0x20

typedef struct function_type * function;

typedef void * function_impl;

typedef void (*function_call_impl)(function, va_list);

function function_create(char * name, argument_list signature);

char * function_get_name(function func);

argument_list function_get_signature(function func);

function_impl function_get_impl(function func);

void function_call(function func, va_list va);

void function_destroy(function func);

#ifdef __cplusplus
}
#endif

#endif // FUNCTION_H
