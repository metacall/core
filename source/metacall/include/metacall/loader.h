#ifndef METACALL_LOADER_H
#define METACALL_LOADER_H 1

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

METACALL_API int load(char * name);

METACALL_API int load_path(char * path);

METACALL_API int unload(void);

#ifdef __cplusplus
}
#endif

#endif // METACALL_LOADER_H
