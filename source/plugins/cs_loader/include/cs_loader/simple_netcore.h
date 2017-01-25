#pragma once
#ifndef _SIMPLE_NETCORE_H_
#define _SIMPLE_NETCORE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <cs_loader/defs.h>

	struct netcore_handle_type;
	typedef struct netcore_handle_type * netcore_handle;

	netcore_handle simple_netcore_create();

	reflect_function * simple_netcore_get_functions(netcore_handle, int*);

	void  simple_netcore_load_script_from_file(netcore_handle handle, const char *path, const char* file);

	void   simple_netcore_load_script_from_memory(netcore_handle handle, const char * buffer, size_t size);

	execution_result* simple_netcore_invoke(netcore_handle, const char *);

	void simple_netcore_destroy(netcore_handle);

	execution_result* simple_netcore_invoke_with_params(netcore_handle handle, const char *func, parameters * params);

	void simple_netcore_destroy_execution_result(netcore_handle handle, execution_result* er);

#ifdef __cplusplus
}
#endif
#endif