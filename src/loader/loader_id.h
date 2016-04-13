#ifndef LOADER_ID_H
#define LOADER_ID_H

#include <loader/loader_naming.h>

enum loader_id
{
	LOADER_ID_JS = 0x00,
	LOADER_ID_PY = 0x01,
	LOADER_ID_RB = 0x02,

	LOADER_ID_COUNT
};

enum loader_id loader_id_from_name(loader_naming_name name);

enum loader_id loader_id_from_extension(loader_naming_extension extension);

#endif // LOADER_ID_H
