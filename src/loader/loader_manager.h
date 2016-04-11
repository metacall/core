#ifndef LOADER_MANAGER_H
#define LOADER_MANAGER_H

#include <loader/loader_impl.h>

enum loader_manager_id
{
	LOADER_MANAGER_JS = 0x00,
	LOADER_MANAGER_PY = 0x01,
	LOADER_MANAGER_RB = 0x02,

	LOADER_MANAGER_COUNT
};

enum loader_manager_id loader_manager_deduce_id(char * name);

loader_impl loader_manager_get(enum loader_manager_id id);

int loader_manager_size(void);

#endif // LOADER_MANAGER_H
