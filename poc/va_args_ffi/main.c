#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ffi.h>

enum TypeId
{
	TYPE_UINT = 0x00,

	TYPE_COUNT
};

ffi_type *get_type_ptr(enum TypeId type)
{
	static ffi_type * type_map[TYPE_COUNT] =
	{
		&ffi_type_uint
	//	&ffi_type_int
	//	[TYPE_UINT] = &ffi_type_uint,
	//	[TYPE_INT] = &ffi_type_int
	};

	return type_map[type];
}

struct FunctionType
{
	ffi_cif cif;
	void *ptr;
	char *name;
	int arg_count;
	ffi_type **args;
	void **values;
};

int function_create(struct FunctionType *func, void *ptr, char *name, int count, enum TypeId ret_val, ffi_type **args)
{
	if (func && name && count >= 0)
	{
		func->ptr = ptr;
		func->name = name;
		func->arg_count = count;
		func->args = args;
		func->values = malloc(func->arg_count * sizeof(void *));

		if (ffi_prep_cif(&func->cif, FFI_DEFAULT_ABI, count, get_type_ptr(ret_val), func->args) == FFI_OK)
		{
			return 1;
		}
	}

	return 0;
}

void function_call(struct FunctionType *func, ...)
{
	va_list va;

	int arg, ret_val;

	va_start(va, func);

	for (arg = 0; arg < func->arg_count; ++arg)
	{
		if (func->args[arg] == &ffi_type_pointer)
		{
			func->values[arg] = va_arg(va, void*);
		}
	}

	va_end(va);

	ffi_call(&func->cif, (void(*)(void))func->ptr, &ret_val, func->values);
}

void function_destroy(struct FunctionType *func)
{
	if (func->values)
	{
		free(func->values);
	}
}

int main(int argc, char *argv[])
{
	struct FunctionType function;
	//struct FunctionParameterType parameter;

	char *hello = "Hello World!";
	char *bye = "Bye Bye!";

	ffi_type *args[1];

	args[0] = &ffi_type_pointer;

	function_create(&function, &puts, "puts", 1, TYPE_UINT, args);

	// First call
	function_call(&function, &hello);

	// Second call
	function_call(&function, &bye);

	function_destroy(&function);

	return 0;
}
