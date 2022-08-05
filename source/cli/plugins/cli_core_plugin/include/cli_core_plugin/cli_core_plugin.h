#ifndef CLI_CORE_PLUGIN_H
#define CLI_CORE_PLUGIN_H 1

#include <cli_core_plugin/cli_core_plugin_api.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

CLI_CORE_PLUGIN_API int cli_core_plugin(void *loader, void *handle, void *context);

DYNLINK_SYMBOL_EXPORT(cli_core_plugin);

#ifdef __cplusplus
}
#endif

#endif /* CLI_CORE_PLUGIN_H */
