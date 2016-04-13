#include <stdio.h>
#include <loader/loader_py.h>

int main(int argc, char * argv[])
{
	loader_impl loader_py = loader_impl_py();
	loader_handle handle;

	if (loader_py->initialize())
	{
		printf("error: initializing python loader\n");
	}

	if (loader_py->execution_path(NULL))
	{
		printf("error: defining default execution path\n");
	}

	handle = loader_py->load("example.py");

	if (handle == NULL)
	{
		printf("error: loading a python script\n");
	}

	if (loader_py->discover(handle))
	{
		printf("error: discovering a python script\n");
	}

	if (loader_py->clear(handle))
	{
		printf("error: clearing a python script\n");
	}

	if (loader_py->destroy())
	{
		printf("error: destroying python loader\n");
	}

	return 0;
}

