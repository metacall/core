#include <loader/loader_id.h>

#include <string.h>

enum loader_id loader_id_from_name(loader_naming_name name)
{
	char extension[LOADER_NAMING_EXTENSION_SIZE] = { '\0' };

	loader_naming_get_extension(name, extension);

	return loader_id_from_extension(extension);
}

enum loader_id loader_id_from_extension(loader_naming_extension extension)
{
	// todo: hash table
	if (strcmp(extension, "js") == 0)
	{
		return LOADER_ID_JS;
	}
	else if (strcmp(extension, "py") == 0)
	{
		return LOADER_ID_PY;
	}
	else if (strcmp(extension, "rb") == 0)
	{
		return LOADER_ID_RB;
	}

	return LOADER_ID_COUNT;
}
