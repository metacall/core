#include <stdio.h>
#include <loader.h>

int main(int argc, char * argv[])
{
	if (load("example.py"))
	{
		printf("Error loading a python script\n");
	}

	if (load("hello-world.js"))
	{
		printf("Error loading a js script\n");
	}

	if (load_path("dummy"))
	{
		printf("Error loading a path\n");
	}

	if (unload())
	{
		printf("Error when unloading all\n");
	}

	return 0;
}

