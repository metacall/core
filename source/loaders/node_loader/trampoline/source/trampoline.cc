/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#include <trampoline/trampoline.h>

#if (defined(WIN32) || defined(_WIN32)) && (_MSC_VER >= 1200)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	define DELAYIMP_INSECURE_WRITABLE_HOOKS 1
#	include <delayimp.h>
#endif

#include <node_api.h>

#include <stdio.h>
#include <assert.h>

#define NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD(name, func) \
	{ name, 0, func, 0, 0, 0, napi_default, 0 }

/* Win32 Delay Load */
#if (defined(WIN32) || defined(_WIN32)) && (_MSC_VER >= 1200)

	static FARPROC WINAPI node_loader_trampoline_win32_delay_load(unsigned dliNotify, PDelayLoadInfo pdli);

#	if (defined(DELAYLOAD_VERSION) && DELAYLOAD_VERSION >= 0x0200) || (defined(_DELAY_IMP_VER) && (_DELAY_IMP_VER >= 2))
		extern PfnDliHook __pfnDliFailureHook2 = node_loader_trampoline_win32_delay_load;
#	else
		extern PfnDliHook __pfnDliFailureHook = node_loader_trampoline_win32_delay_load;
#	endif

	FARPROC WINAPI node_loader_trampoline_win32_delay_load(unsigned dliNotify, PDelayLoadInfo pdli)
	{
		FARPROC fp_module_register = NULL;

		if (dliNotify == dliFailGetProc)
		{
			LPCTSTR module_handle_lpctstr = "node.dll";

			HMODULE module_handle = GetModuleHandle(module_handle_lpctstr);

			fp_module_register = ::GetProcAddress(module_handle, pdli->dlp.szProcName);
		}

		return fp_module_register;
	}
#endif

napi_value node_loader_trampoline_register(napi_env env, napi_callback_info info)
{
	napi_status status;

	const size_t args_size = 3;
	size_t argc = args_size;

	napi_value args[args_size];
	napi_valuetype valuetype[args_size];

	/* Parse arguments */
	status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

	assert(status == napi_ok);

	if (argc < args_size)
	{
		napi_throw_type_error(env, nullptr, "Wrong number of arguments");

		return nullptr;
	}

	/* Parse argument value type */
	status = napi_typeof(env, args[0], &valuetype[0]);

	assert(status == napi_ok);

	status = napi_typeof(env, args[1], &valuetype[1]);

	assert(status == napi_ok);

	status = napi_typeof(env, args[2], &valuetype[2]);

	assert(status == napi_ok);

	if (valuetype[0] != napi_string || valuetype[1] != napi_string || valuetype[2] != napi_object)
	{
		napi_throw_type_error(env, nullptr, "Wrong arguments type");

		return nullptr;
	}

	/* Get pointer in string format (TODO: Review size for 32-64bit) */
	const size_t ptr_str_size = 16 + 1;
	size_t ptr_str_size_copied = 0;
	char ptr_str[ptr_str_size];
	void * ptr = NULL;
	void * node_impl_ptr;
	node_loader_trampoline_register_ptr register_ptr = NULL;

	/* Get node impl pointer */
	status = napi_get_value_string_utf8(env, args[0], ptr_str, ptr_str_size, &ptr_str_size_copied);

	assert(status == napi_ok);

	/* Convert the string to pointer type */
	sscanf(ptr_str, "%p", &ptr);

	/* Cast to function */
	node_impl_ptr = ptr;

	/* Get register pointer */
	status = napi_get_value_string_utf8(env, args[1], ptr_str, ptr_str_size, &ptr_str_size_copied);

	assert(status == napi_ok);

	/* Convert the string to pointer type */
	sscanf(ptr_str, "%p", &ptr);

	/* Cast to function */
	register_ptr = (node_loader_trampoline_register_ptr)ptr;

	/* Get function table object */
	napi_value function_table_object;

	status = napi_coerce_to_object(env, args[2], &function_table_object);

	assert(status == napi_ok);

	/* Register function table object */
	(void)register_ptr(node_impl_ptr, static_cast<void *>(env), static_cast<void *>(function_table_object));

	/* TODO: Return */
	napi_value ptr_value;

	status = napi_create_string_utf8(env, ptr_str, ptr_str_size_copied, &ptr_value);

	assert(status == napi_ok);

	return ptr_value;
}

#if 0
/* TODO: This data must be binded to the promise, not here */
future_resolve_callback resolve_callback;
future_reject_callback reject_callback;

napi_value future_node_on_resolve(napi_env env, napi_callback_info info)
{
	loader_impl_node node_impl;

	size_t argc;

	napi_value argv[1], this_arg, result;

	void * data;

	napi_status status;

	value arg, ret;

	napi_handle_scope handle_scope;

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Retrieve the arguments and bind data */
	status = napi_get_cb_info(env, info, &argc, &argv[0], &this_arg, &data);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		/* TODO: Error handling */
	}

	node_impl = static_cast<loader_impl_node>(data);

	if (node_impl->resolve_callback == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, argv[0]);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the resolve callback */
	ret = node_impl->resolve_callback(arg, NULL /* TODO: data*/);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	result = node_loader_impl_value_to_napi(node_impl, env, ret);

	/* Close scope */
	status = napi_close_handle_scope(node_impl->env, handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}

napi_value future_node_on_reject(napi_env env, napi_callback_info info)
{
	loader_impl_node node_impl;

	size_t argc;

	napi_value argv[1], this_arg, result;

	void * data;

	napi_status status;

	value arg, ret;

	napi_handle_scope handle_scope;

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Retrieve the arguments and bind data */
	status = napi_get_cb_info(env, info, &argc, &argv[0], &this_arg, &data);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		/* TODO: Error handling */
	}

	node_impl = static_cast<loader_impl_node>(data);

	if (node_impl->reject_callback == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, argv[0]);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the resolve callback */
	ret = node_impl->reject_callback(arg, NULL /* TODO: data*/);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	result = node_loader_impl_value_to_napi(node_impl, env, ret);

	/* Close scope */
	status = napi_close_handle_scope(node_impl->env, handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}
#endif


napi_value node_loader_trampoline_register_initialize(napi_env env, napi_value exports)
{
	napi_status status;

	/* Declare register function */
	napi_property_descriptor desc = NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("register", node_loader_trampoline_register);

	status = napi_define_properties(env, exports, 1, &desc);

	assert(status == napi_ok);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, node_loader_trampoline_register_initialize)
