#include <metacall/loader.h>
#include <metacall/loader/loader_manager.h>

#include <stdio.h>

int load(char * name)
{
	enum loader_id id = loader_id_from_name(name);

	loader_impl loader = loader_manager_get(id);

	if (loader != NULL)
	{

		printf("debug: loader %s is currently loaded\n", name);

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
