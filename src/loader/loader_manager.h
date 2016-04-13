#ifndef LOADER_MANAGER_H
#define LOADER_MANAGER_H

#include <loader/loader_id.h>
#include <loader/loader_impl.h>

#include <stddef.h>

loader_impl loader_manager_get(enum loader_id id);

size_t loader_manager_size(void);

#endif // LOADER_MANAGER_H
