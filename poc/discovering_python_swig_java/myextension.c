#include "myextension.h"

#include <function.h>
#include <ruby_backend.h>
#include <python_backend.h>

void run()
{
	// Run ffi example
	ffi_run_example();

	// Run ruby script
	ruby_run_example();

	// Run python script
	python_run_example();
}
