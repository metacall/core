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
#include <node_api.h>

#define NODE_LOADER_PROCESS_TITLE "node-loader-testd"

#ifndef container_of
#	define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#endif

using namespace v8;

typedef struct loader_impl_node_type
{
	uv_thread_t thread_id;
	uv_loop_t * thread_loop;
	uv_mutex_t mutex_start;
	uv_cond_t cond_start;
	uv_async_t async_load_from_file;
	uv_async_t async_call;
	uv_async_t async_destroy;

} * loader_impl_node;

typedef struct loader_impl_async_load_from_file_type
{
	const loader_naming_path * paths;
	size_t size;

} * loader_impl_async_load_from_file;

void node_loader_impl_async_call(uv_async_t * async);

void node_loader_impl_async_load_from_file(uv_async_t * async);

void node_loader_impl_walk(uv_handle_t * handle, void * data);

void node_loader_impl_async_destroy(uv_async_t * async);

void node_loader_impl_async_call(uv_async_t * async)
{
	/* TODO: Parameter will be a reflect function type in the future */
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	(void)node_impl;
}

void node_loader_impl_async_load_from_file(uv_async_t * async)
{
	loader_impl_async_load_from_file async_data = static_cast<loader_impl_async_load_from_file>(async->data);

	printf("%s\n", async_data->paths[0]);
}

void node_loader_impl_thread(void * data)
{
	/* TODO: Do a workaround with app title */
	char * argv[] = { "node-loader-testd", "scripts/nod.js", NULL };

	int argc = 2;

	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));

	node_impl->thread_loop = uv_default_loop();

	/* Initialize load from file signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_load_from_file, &node_loader_impl_async_load_from_file);

	/* Initialize call signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_call, &node_loader_impl_async_call);

	/* Initialize destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_destroy, &node_loader_impl_async_destroy);

	/* Signal start condition */
	uv_mutex_lock(&node_impl->mutex_start);

	uv_cond_signal(&node_impl->cond_start);

	uv_mutex_unlock(&node_impl->mutex_start);

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

	uv_cond_init(&node_impl->cond_start);

	uv_mutex_init(&node_impl->mutex_start);

	/* Create NodeJS thread */
	uv_thread_create(&node_impl->thread_id, node_loader_impl_thread, &node_impl);

	/* Wait until start has been launch */
	uv_mutex_lock(&node_impl->mutex_start);

	uv_cond_wait(&node_impl->cond_start, &node_impl->mutex_start);

	uv_mutex_unlock(&node_impl->mutex_start);

	/* TODO: Waiting for script load (there is a posibility of produce */
	/* a race condition if something is executed before V8 initialization) */
	usleep(4000000);

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
	static int mock_file_handle = 0;

	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return NULL;
	}

	struct loader_impl_async_load_from_file_type async_data =
	{
		paths,
		size
	};

	node_impl->async_load_from_file.data = static_cast<void *>(&async_data);

	uv_async_send(&node_impl->async_load_from_file);

	/* TODO: Wait until module is loaded or return the promise ? */
	usleep(4000000);

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

void node_loader_impl_walk(uv_handle_t * handle, void * arg)
{
	(void)arg;

	if (!uv_is_closing(handle))
	{
		uv_close(handle, NULL);
	}
}

void node_loader_impl_async_destroy(uv_async_t * async)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	uv_stop(node_impl->thread_loop);

	uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);

	while (uv_run(node_impl->thread_loop, UV_RUN_DEFAULT) != 0);

	if (uv_loop_alive(node_impl->thread_loop) != 0)
	{
		/* TODO: Error message */
	}

	if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
	{
		/* TODO: Error message */
	}
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return 1;
	}

	/* When running on interpreter, closes the stdin handle and */
	/* gracefully stops the node event loop */
	/*
	fclose(stdin);
	*/

	uv_mutex_destroy(&node_impl->mutex_start);

	uv_cond_destroy(&node_impl->cond_start);

	node_impl->async_destroy.data = static_cast<void *>(&node_impl);

	uv_async_send(&node_impl->async_destroy);

	uv_close(reinterpret_cast<uv_handle_t*>(&node_impl->async_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t*>(&node_impl->async_call), NULL);

	uv_close(reinterpret_cast<uv_handle_t*>(&node_impl->async_load_from_file), NULL);

	uv_thread_join(&node_impl->thread_id);

	free(node_impl);

	return 0;
}
