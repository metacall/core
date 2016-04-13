#include <stdio.h>
#include <loader.h>

int main(int argc, char * argv[])
{
	if (load("example.py"))
	{
		printf("error: loading a python script\n");
	}

	if (load("hello-world.js"))
	{
		printf("error: loading a js script\n");
	}

	if (load_path("dummy"))
	{
		printf("error: loading a path\n");
	}

	if (unload())
	{
		printf("error: when unloading all\n");
	}

	return 0;
}

