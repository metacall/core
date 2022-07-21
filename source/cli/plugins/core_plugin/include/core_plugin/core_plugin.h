#ifndef CORE_PLUGIN_H
#define CORE_PLUGIN_H 1

#include <core_plugin/core_plugin_api.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

CORE_PLUGIN_API int core_plugin(void *loader, void *handle, void *context);

DYNLINK_SYMBOL_EXPORT(core_plugin);

#ifdef __cplusplus
}
#endif

#endif /* CORE_PLUGIN_H */
