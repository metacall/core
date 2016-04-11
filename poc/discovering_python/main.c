#include <function.h>
#include <ruby_backend.h>
#include <python_backend.h>

int main(int argc, char *argv[])
{
	// Run ffi example
	ffi_run_example();

	// Run ruby script
	ruby_run_example();

	// Run python script
	python_run_example();

	return 0;
}
