#ifndef LOADER_MANAGER_H
#define LOADER_MANAGER_H 1

#include <loader/loader_id.h>
#include <loader/loader_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

loader_impl loader_manager_get(enum loader_id id);

size_t loader_manager_size(void);

#ifdef __cplusplus
}
#endif

#endif // LOADER_MANAGER_H
