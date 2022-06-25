#ifndef LOAD_EXTENSIONS_H
#define LOAD_EXTENSIONS_H 1

#include <dynlink/dynlink.h>
#include <load_extensions/load_extensions_api.h>

#ifdef __cplusplus
extern "C" {
#endif

LOAD_EXTENSIONS_API void load_extensions(void *loader, void *context);

DYNLINK_SYMBOL_EXPORT(load_extensions);

#ifdef __cplusplus
}
#endif

#endif //extension.h
