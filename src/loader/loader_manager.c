#include <loader/loader_manager.h>

#include <loader/loader_js.h>
#include <loader/loader_py.h>
#include <loader/loader_rb.h>

#include <stdio.h>
#include <string.h>

#define LOADER_MANAGER_EXTENSION_SIZE 0xFF

enum loader_manager_id loader_manager_deduce_id(char * name)
{
	char extension[LOADER_MANAGER_EXTENSION_SIZE] = { '\0' };

	/*
	// from right to left

	int i, count, length = strlen(name);

	for (i = length - 1, count = 0; i >= 0 && name[i] != '.'; --i, ++count)
	{
		printf("extension[%d] = name[%d] (%c)\n", count, i, name[i]);

		extension[count] = name[i];
	}
	*/

	// from left to right

	int i, count, length;

	for (i = 0, count = 0, length = strlen(name); i < length; ++i)
	{
		extension[count++] = name[i];

		if (name[i] == '.')
		{
			count = 0;
		}
	}

	extension[count] = '\0';

	printf("loader manager extension: %s\n", extension);

	// todo: hash table
	if (strcmp(extension, "js") == 0)
	{
		return LOADER_MANAGER_JS;
	}
	else if (strcmp(extension, "py") == 0)
	{
		return LOADER_MANAGER_PY;
	}
	else if (strcmp(extension, "rb") == 0)
	{
		return LOADER_MANAGER_RB;
	}

	return LOADER_MANAGER_COUNT;
}

loader_impl loader_manager_get(enum loader_manager_id id)
{
	static loader_impl_singleton loader_manager_storage[LOADER_MANAGER_COUNT] =
	{
		&loader_impl_js,	// LOADER_MANAGER_JS
		&loader_impl_py,	// LOADER_MANAGER_PY
		&loader_impl_rb		// LOADER_MANAGER_RB
	};

	if (id < LOADER_MANAGER_COUNT && loader_manager_storage[id] != NULL);
	{
		return loader_manager_storage[id]();
	}

	return NULL;
}

int loader_manager_size()
{
	return LOADER_MANAGER_COUNT;
}
