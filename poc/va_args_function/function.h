#ifndef FUNCTION_H
#define FUNCTION_H 1

#include <stdarg.h>
#include <ffi.h>

enum TypeId
{
	TYPE_UINT = 0x00,

	TYPE_COUNT
};

struct FunctionType
{
	ffi_cif cif;
	void *ptr;
	char *name;
	int arg_count;
	ffi_type **args;
	void **values;
};

ffi_type *type_ptr(enum TypeId type);

int function_create(struct FunctionType *func, void *ptr, char *name, int count, enum TypeId ret_val, ffi_type **args);

void function_call(struct FunctionType *func, ...);

void function_destroy(struct FunctionType *func);

#endif // FUNCTION_H
