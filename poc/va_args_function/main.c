#include <stdio.h>
#include <function.h>

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

