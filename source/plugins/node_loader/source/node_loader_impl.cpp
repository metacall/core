/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <node_loader/node_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <cstdlib>
#include <cstring>

#include <new>
#include <string>
#include <fstream>
#include <streambuf>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 6.2.414.50 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <uv.h>

#include <node.h>

#define NODE_LOADER_PROCESS_TITLE "metacall_node_loader"

using namespace v8;

typedef struct loader_impl_node_type
{
	uv_thread_t thread_id;
	uv_loop_t * thread_loop;

} * loader_impl_node;

void node_loader_impl_thread(void * data)
{
	char app_title[sizeof(NODE_LOADER_PROCESS_TITLE)];

	/* TODO: Do a workaround with app title */
	char * argv[] = { app_title, NULL };

	int argc = 1;

	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));

	std::strncpy(app_title, NODE_LOADER_PROCESS_TITLE, sizeof(NODE_LOADER_PROCESS_TITLE) - 1);

	node_impl->thread_loop = uv_default_loop();

	/* Start NodeJS runtime */
	node::Start(argc, reinterpret_cast<char **>(argv));
}

loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_node node_impl;

	(void)impl;
	(void)config;

	if (log_copy(host->log) != 0)
	{
		return NULL;
	}

	node_impl = new loader_impl_node_type();

	if (node_impl == nullptr)
	{
		return NULL;
	}

	uv_thread_create(&node_impl->thread_id, node_loader_impl_thread, &node_impl);

	return node_impl;
}

int node_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return 0;
}

loader_handle node_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	/* TODO */

	(void)impl;
	(void)paths;
	(void)size;

	return NULL;
}

loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	/* TODO */

	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	return NULL;
}

loader_handle node_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return NULL;
}

int node_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	/* TODO */

	(void)impl;
	(void)handle;

	return 0;
}

int node_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* TODO */

	(void)impl;
	(void)handle;
	(void)ctx;

	return 0;
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return 1;
	}

	uv_stop(node_impl->thread_loop);

	uv_thread_join(&node_impl->thread_id);

	free(node_impl);

	return 0;
}
