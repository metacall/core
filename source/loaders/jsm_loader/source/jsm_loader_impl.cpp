/*
 *	Loader Library by Parra Studios
 *	A plugin for loading javascript code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <jsm_loader/jsm_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <stdlib.h>

#include <js/Initialization.h>
#include <js/RootingAPI.h>

/* Assume we can't use more than 5e5 bytes of C stack by default */
#if (defined(DEBUG) && defined(__SUNPRO_CC)) || defined(JS_CPU_SPARC)
/*
	 * Sun compiler uses a larger stack space for js::Interpret() with
	 * debug (use a bigger gMaxStackSize to make "make check" happy)
	*/
	#define JSM_STACK_MAX_SIZE size_t(5000000)
#else
	#define JSM_STACK_MAX_SIZE size_t(500000)
#endif

/*using namespace JS;*/

typedef struct loader_impl_jsm_type
{
	JSRuntime *runtime;
	JSContext *cx;

} * loader_impl_jsm;

typedef struct loader_impl_jsm_handle_type
{
	void *todo;

} * loader_impl_jsm_handle;

int function_jsm_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_jsm_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_jsm_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_jsm_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_jsm_singleton()
{
	static struct function_interface_type jsm_interface = {
		&function_jsm_interface_create,
		&function_jsm_interface_invoke,
		&function_jsm_interface_await,
		&function_jsm_interface_destroy
	};

	return &jsm_interface;
}

const JSClass *jsm_global_class(void)
{
	static const JSClass global_class = {
		"global", JSCLASS_GLOBAL_FLAGS,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL,
		JS_GlobalObjectTraceHook
	};

	return &global_class;
}

void jsm_report_error(JSContext *cx, const char *msg, JSErrorReport *report)
{
	const char *file_name = (report->filename != NULL) ? report->filename : "[no filename]";

	(void)cx;

	log_write("metacall", LOG_LEVEL_ERROR, "%s:%u:\n%s", file_name, (unsigned int)report->lineno, msg);
}

loader_impl_data jsm_loader_impl_initialize(loader_impl impl, configuration config)
{
	(void)impl;
	(void)config;

	if (JS_Init() == true)
	{
		loader_impl_jsm jsm_impl = static_cast<loader_impl_jsm>(malloc(sizeof(struct loader_impl_jsm_type)));

		if (jsm_impl != NULL)
		{
			jsm_impl->runtime = JS_NewRuntime(size_t(8 * 1024 * 1024));

			if (jsm_impl->runtime != NULL)
			{
				const size_t stack_max_size = JSM_STACK_MAX_SIZE;

				JS_SetErrorReporter(jsm_impl->runtime, &jsm_report_error);

				JS_SetNativeStackQuota(jsm_impl->runtime, stack_max_size);

				jsm_impl->cx = JS_NewContext(jsm_impl->runtime, 8192);

				if (jsm_impl->cx != NULL)
				{
					/* Register initialization */
					loader_initialization_register(impl);

					return (loader_impl_data)jsm_impl;
				}

				JS_DestroyRuntime(jsm_impl->runtime);
			}

			free(jsm_impl);
		}

		JS_ShutDown();
	}

	return NULL;
}

int jsm_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle jsm_loader_impl_load(loader_impl impl, const loader_path path, loader_name name)
{
	loader_impl_jsm_handle jsm_handle = static_cast<loader_impl_jsm_handle>(malloc(sizeof(struct loader_impl_jsm_handle_type)));

	(void)impl;
	(void)path;
	(void)name;

	if (jsm_handle != NULL)
	{
		return (loader_handle)jsm_handle;
	}

	return NULL;
}

int jsm_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_jsm_handle jsm_handle = (loader_impl_jsm_handle)handle;

	(void)impl;

	if (jsm_handle != NULL)
	{
		free(jsm_handle);

		return 0;
	}

	return 1;
}

int jsm_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* loader_impl_jsm_handle jsm_handle = (loader_impl_jsm_handle)handle; */

	(void)impl;
	(void)handle;
	(void)ctx;

	return 0;
}

int jsm_loader_impl_destroy(loader_impl impl)
{
	loader_impl_jsm jsm_impl = (loader_impl_jsm)loader_impl_get(impl);

	if (jsm_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl);

		/* Destroy Spider Monkey */
		if (jsm_impl->cx != NULL)
		{
			JS_DestroyContext(jsm_impl->cx);

			jsm_impl->cx = NULL;
		}

		if (jsm_impl->runtime != NULL)
		{
			JS_DestroyRuntime(jsm_impl->runtime);

			jsm_impl->runtime = NULL;
		}

		free(jsm_impl);

		JS_ShutDown();

		return 0;
	}

	return 1;
}
