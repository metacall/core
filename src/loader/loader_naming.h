#ifndef LOADER_NAMING_H
#define LOADER_NAMING_H

#define LOADER_NAMING_NAME_SIZE 0xFF
#define LOADER_NAMING_EXTENSION_SIZE 0xFF

typedef char loader_naming_name[LOADER_NAMING_NAME_SIZE];
typedef char loader_naming_extension[LOADER_NAMING_NAME_SIZE];

int loader_naming_get_name(loader_naming_name path, loader_naming_name name);

int loader_naming_get_extension(loader_naming_name path, loader_naming_extension extension);

#endif // LOADER_NAMING_H
