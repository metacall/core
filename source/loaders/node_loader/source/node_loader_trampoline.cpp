/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#include <node_loader/node_loader_impl.h>
#include <node_loader/node_loader_trampoline.h>

#include <stdio.h> /* TODO: Improve this trick */

#define NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD(name, func) \
	{ name, 0, func, 0, 0, 0, napi_default, 0 }

typedef void * (*future_resolve_callback)(void *, void *);
typedef void * (*future_reject_callback)(void *, void *);

typedef napi_value (*future_resolve_trampoline)(void *, napi_env, future_resolve_callback, napi_value, napi_value, void *);
typedef napi_value (*future_reject_trampoline)(void *, napi_env, future_reject_callback, napi_value, napi_value, void *);

typedef struct loader_impl_async_future_await_trampoline_type
{
	loader_impl_node node_impl;
	future_resolve_trampoline resolve_trampoline;
	future_reject_trampoline reject_trampoline;
	future_resolve_callback resolve_callback;
	future_resolve_callback reject_callback;
	void * context;

} * loader_impl_async_future_await_trampoline;

/* This is for holding the node loader reference for later
 * on passing it to the destroy and requested_destroy methods
 */
static loader_impl_node *node_impl_ptr = NULL;

/* TODO: This is a trick, probably with the current architecture we can
 * pass the pointer through the trampoline itself as a wrapped reference
 */
static void node_loader_trampoline_parse_pointer(napi_env env, napi_value v, void **ptr)
{
	const size_t ptr_str_size = (sizeof(void *) * 2) + 1;
	size_t ptr_str_size_copied = 0;
	char ptr_str[ptr_str_size];
	napi_status status = napi_get_value_string_utf8(env, v, ptr_str, ptr_str_size, &ptr_str_size_copied);

	node_loader_impl_exception(env, status);

	/* Convert the string to pointer type */
	sscanf(ptr_str, "%p", ptr);
}

napi_value node_loader_trampoline_register(napi_env env, napi_callback_info info)
{
	napi_status status;

	const size_t args_size = 3;
	size_t argc = args_size;

	napi_value args[args_size];
	napi_valuetype valuetype[args_size];

	/* Parse arguments */
	status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	if (argc != args_size)
	{
		napi_throw_type_error(env, nullptr, "Wrong number of arguments");

		return nullptr;
	}

	/* Parse argument value type */
	status = napi_typeof(env, args[0], &valuetype[0]);

	node_loader_impl_exception(env, status);

	status = napi_typeof(env, args[1], &valuetype[1]);

	node_loader_impl_exception(env, status);

	status = napi_typeof(env, args[2], &valuetype[2]);

	node_loader_impl_exception(env, status);

	if (valuetype[0] != napi_string || valuetype[1] != napi_string || valuetype[2] != napi_object)
	{
		napi_throw_type_error(env, nullptr, "Wrong arguments type");

		return nullptr;
	}

	loader_impl_node node_impl = NULL;
	node_loader_trampoline_register_ptr register_ptr = NULL;

	/* Get node impl pointer */
	node_loader_trampoline_parse_pointer(env, args[0], (void **)&node_impl);

	/* Get register function pointer */
	node_loader_trampoline_parse_pointer(env, args[1], (void **)&register_ptr);

	/* Get function table object */
	napi_value function_table_object;

	status = napi_coerce_to_object(env, args[2], &function_table_object);

	node_loader_impl_exception(env, status);

	/* Register function table object */
	(void)register_ptr(node_impl, static_cast<void *>(env), static_cast<void *>(function_table_object));

	/* Store the node impl reference into a pointer so we can use it later on in the destroy mechanism */
	*node_impl_ptr = node_impl;

	return nullptr;
}

napi_value node_loader_trampoline_resolve(napi_env env, napi_callback_info info)
{
	napi_status status;

	const size_t args_size = 2;
	size_t argc = args_size;
	napi_value recv;

	napi_value args[args_size];
	napi_valuetype valuetype[args_size];

	/* Parse arguments */
	status = napi_get_cb_info(env, info, &argc, args, &recv, nullptr);

	node_loader_impl_exception(env, status);

	if (argc != args_size)
	{
		napi_throw_type_error(env, nullptr, "Wrong number of arguments");

		return nullptr;
	}

	/* Parse argument value type */
	status = napi_typeof(env, args[0], &valuetype[0]);

	node_loader_impl_exception(env, status);

	status = napi_typeof(env, args[1], &valuetype[1]);

	node_loader_impl_exception(env, status);

	if (valuetype[0] != napi_object /* TODO: check valuetype[1] */)
	{
		napi_throw_type_error(env, nullptr, "Wrong arguments type");

		return nullptr;
	}

	/* Unwrap trampoline pointer */
	void * result;

	status = napi_unwrap(env, args[0], &result);

	node_loader_impl_exception(env, status);

	if (result == NULL)
	{
		return NULL;
	}

	/* Execute the callback */
	loader_impl_async_future_await_trampoline trampoline = static_cast<loader_impl_async_future_await_trampoline>(result);

	return trampoline->resolve_trampoline(trampoline->node_impl, env, trampoline->resolve_callback, recv, args[1], trampoline->context);
}

napi_value node_loader_trampoline_reject(napi_env env, napi_callback_info info)
{
	napi_status status;

	const size_t args_size = 2;
	size_t argc = args_size;
	napi_value recv;

	napi_value args[args_size];
	napi_valuetype valuetype[args_size];

	/* Parse arguments */
	status = napi_get_cb_info(env, info, &argc, args, &recv, nullptr);

	node_loader_impl_exception(env, status);

	if (argc != args_size)
	{
		napi_throw_type_error(env, nullptr, "Wrong number of arguments");

		return nullptr;
	}

	/* Parse argument value type */
	status = napi_typeof(env, args[0], &valuetype[0]);

	node_loader_impl_exception(env, status);

	status = napi_typeof(env, args[1], &valuetype[1]);

	node_loader_impl_exception(env, status);

	if (valuetype[0] != napi_object /* TODO: check valuetype[1] */)
	{
		napi_throw_type_error(env, nullptr, "Wrong arguments type");

		return nullptr;
	}

	/* Unwrap trampoline pointer */
	void * result;

	status = napi_unwrap(env, args[0], &result);

	node_loader_impl_exception(env, status);

	if (result == NULL)
	{
		return NULL;
	}

	/* Execute the callback */
	loader_impl_async_future_await_trampoline trampoline = static_cast<loader_impl_async_future_await_trampoline>(result);

	return trampoline->reject_trampoline(trampoline->node_impl, env, trampoline->reject_callback, recv, args[1], trampoline->context);
}

napi_value node_loader_trampoline_destroy(napi_env env, napi_callback_info info)
{
	(void)info;

	node_loader_impl_destroy_safe(*node_impl_ptr, env);

	return nullptr;
}

napi_value node_loader_trampoline_requested_destroy(napi_env env, napi_callback_info info)
{
	(void)info;

	napi_value v = nullptr;
	bool requested_destroy = node_loader_impl_requested_destroy(*node_impl_ptr);
	napi_status status = napi_get_boolean(env, requested_destroy, &v);
	node_loader_impl_exception(env, status);
	return v;
}

napi_value node_loader_trampoline_initialize(napi_env env, napi_value exports)
{
	napi_status status;

	/* Declare register function */
	napi_property_descriptor desc[] =
	{
		NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("register", node_loader_trampoline_register),
		NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("resolve", node_loader_trampoline_resolve),
		NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("reject", node_loader_trampoline_reject),
		NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("destroy", node_loader_trampoline_destroy),
		NODE_LOADER_TRAMPOLINE_DECLARE_NAPI_METHOD("requested_destroy", node_loader_trampoline_requested_destroy)
	};

	status = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

	node_loader_impl_exception(env, status);

	return exports;
}
