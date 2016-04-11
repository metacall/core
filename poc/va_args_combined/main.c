#include <stdio.h>
#include <function.h>
#include <ruby_backend.h>
#include <python_backend.h>

int main(int argc, char *argv[])
{
	struct FunctionType function;
	//struct FunctionParameterType parameter;

	char *hello = "Hello World!";
	char *bye = "Bye Bye!";

	ffi_type *args[1];

	// Create ffi
	args[0] = &ffi_type_pointer;

	function_create(&function, &puts, "puts", 1, TYPE_UINT, args);

	// First call
	function_call(&function, &hello);

	// Second call
	function_call(&function, &bye);

	// Destroy
	function_destroy(&function);

	// Run ruby script
	ruby_run_example();

	// Run python script
	python_run_example();

	return 0;
}
