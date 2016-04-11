#include <loader.h>
#include <loader/loader_manager.h>

#include <stdio.h>

int load(char * name)
{
	enum loader_manager_id id = loader_manager_deduce_id(name);

	loader_impl loader = loader_manager_get(id);

	if (loader != NULL)
	{

		printf("We have a loader!\n");

		return 0;
	}

	return 1;
}

int load_path(char * path)
{

	return 0;
}

int unload()
{

	return 0;
}
