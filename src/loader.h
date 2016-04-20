#ifndef LOADER_H
#define LOADER_H 1

#ifdef __cplusplus
extern "C" {
#endif

int load(char * name);

int load_path(char * path);

int unload(void);

#ifdef __cplusplus
}
#endif

#endif // LOADER_H
