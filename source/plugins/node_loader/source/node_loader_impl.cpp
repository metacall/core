/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
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
	uv_async_t async_destroy;

} * loader_impl_node;

void node_loader_impl_async_destroy(uv_async_t * async)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	uv_stop(node_impl->thread_loop);
}

void node_loader_impl_thread(void * data)
{
	char app_title[sizeof(NODE_LOADER_PROCESS_TITLE)];

	/* TODO: Do a workaround with app title */
	char * argv[] = { app_title, NULL };

	int argc = 1;

	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));

	std::strncpy(app_title, NODE_LOADER_PROCESS_TITLE, sizeof(NODE_LOADER_PROCESS_TITLE) - 1);

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

	node_impl->thread_loop = uv_default_loop();

	uv_async_init(node_impl->thread_loop, &node_impl->async_destroy, &node_loader_impl_async_destroy);

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
	static int mock_file_handle = 0;

	(void)impl;
	(void)paths;
	(void)size;

	return &mock_file_handle;
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

void node_loader_impl_close(uv_handle_t * handle)
{
	if (handle != NULL)
	{
		delete handle;
	}
}

void node_loader_impl_walk(uv_handle_t * handle, void * data)
{
	(void)data;

	uv_close(handle, node_loader_impl_close);
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return 1;
	}

	node_impl->async_destroy.data = static_cast<void *>(&node_impl);

	uv_async_send(&node_impl->async_destroy);

	uv_thread_join(&node_impl->thread_id);

	uv_close(reinterpret_cast<uv_handle_t*>(&node_impl->async_destroy), NULL);

	/* Run as default to leave libuv clean up */
	uv_run(node_impl->thread_loop, UV_RUN_DEFAULT);

	if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
	{
		uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);
	}

	free(node_impl);

	return 0;
}
