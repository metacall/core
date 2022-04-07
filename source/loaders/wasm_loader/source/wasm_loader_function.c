#include <wasm_loader/wasm_loader_function.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <wasm.h>

struct loader_impl_wasm_function_type
{
	const wasm_func_t *func;
};

static function_return function_wasm_interface_invoke(function func, function_impl impl, function_args args, size_t args_size);
static void function_wasm_interface_destroy(function func, function_impl impl);

function_interface function_wasm_singleton(void)
{
	static struct function_interface_type wasm_function_interface = {
		NULL,
		&function_wasm_interface_invoke,
		// Threads are only in Phase 2 of the standardization process
		// (see https://github.com/WebAssembly/proposals) and are currently
		// not fully implemented in Wasmtime
		// (see https://docs.wasmtime.dev/stability-wasm-proposals-support.html)
		NULL,
		&function_wasm_interface_destroy
	};

	return &wasm_function_interface;
}

loader_impl_wasm_function loader_impl_wasm_function_create(const wasm_func_t *func)
{
	loader_impl_wasm_function func_impl = malloc(sizeof(struct loader_impl_wasm_function_type));
	if (func_impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate memory for function handle");
		return NULL;
	}

	// Ugly hack to subvert the type system and initialize a const member
	*(wasm_func_t **)&func_impl->func = (wasm_func_t *)func;

	return func_impl;
}

static int reflect_to_wasm_type(value val, wasm_val_t *ret)
{
	type_id val_type = value_type_id(val);
	if (val_type == TYPE_INT)
	{
		*ret = (wasm_val_t)WASM_I32_VAL(value_to_int(val));
	}
	else if (val_type == TYPE_LONG)
	{
		*ret = (wasm_val_t)WASM_I64_VAL(value_to_long(val));
	}
	else if (val_type == TYPE_FLOAT)
	{
		*ret = (wasm_val_t)WASM_F32_VAL(value_to_float(val));
	}
	else if (val_type == TYPE_DOUBLE)
	{
		*ret = (wasm_val_t)WASM_F64_VAL(value_to_double(val));
	}
	else
	{
		return 1;
	}

	return 0;
}

static value wasm_to_reflect_type(wasm_val_t val)
{
	if (val.kind == WASM_I32)
	{
		return value_create_int(val.of.i32);
	}
	else if (val.kind == WASM_I64)
	{
		return value_create_long(val.of.i64);
	}
	else if (val.kind == WASM_F32)
	{
		return value_create_float(val.of.f32);
	}
	else if (val.kind == WASM_F64)
	{
		return value_create_double(val.of.f64);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Unrecognized Wasm value kind (kind %d)", val.kind);
		return NULL;
	}
}

static value wasm_results_to_reflect_type(const wasm_val_vec_t *results)
{
	if (results->size == 1)
	{
		return wasm_to_reflect_type(results->data[0]);
	}
	else
	{
		value values[results->size];
		for (size_t idx = 0; idx < results->size; idx++)
		{
			values[idx] = wasm_to_reflect_type(results->data[idx]);
		}

		return value_create_array(values, results->size);
	}
}

static void log_trap(const wasm_trap_t *trap)
{
	wasm_byte_vec_t message;
	wasm_trap_message(trap, &message);
	log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Executed trap with message \"%s\"", message.data);
	wasm_byte_vec_delete(&message);
}

static value call_func(const signature sig, const wasm_func_t *func, const wasm_val_vec_t args)
{
	// No way to check if vector allocation fails
	wasm_val_vec_t results;
	wasm_val_vec_new_uninitialized(&results, wasm_func_result_arity(func));

	wasm_trap_t *trap = wasm_func_call(func, &args, &results);

	value ret = NULL;
	if (trap != NULL)
	{
		// BEWARE: Wasmtime executes an undefined instruction in order to
		// transfer control to its trap handlers, which can cause Valgrind and
		// other debuggers to act out. In Valgrind, this can be suppressed
		// using the `--sigill-diagnostics=no` flag. Other debuggers support
		// similar behavior.
		// See https://github.com/bytecodealliance/wasmtime/issues/3061 for
		// more information
		log_trap(trap);
		wasm_trap_delete(trap);
	}
	else if (signature_get_return(sig) != NULL)
	{
		ret = wasm_results_to_reflect_type(&results);
	}

	wasm_val_vec_delete(&results);
	return ret;
}

static function_return function_wasm_interface_invoke(function func, function_impl impl, function_args args, size_t args_size)
{
	loader_impl_wasm_function wasm_func = (loader_impl_wasm_function)impl;
	signature sig = function_signature(func);

	if (args_size != signature_count(sig))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Invalid number of arguments (%d expected, %d given)", args_size, signature_count(sig));
		return NULL;
	}

	if (args_size == 0)
	{
		const wasm_val_vec_t args_vec = WASM_EMPTY_VEC;

		return call_func(sig, wasm_func->func, args_vec);
	}
	else
	{
		wasm_val_t wasm_args[args_size];

		for (size_t idx = 0; idx < args_size; idx++)
		{
			type param_type = signature_get_type(sig, idx);
			type_id param_type_id = type_index(param_type);
			type_id arg_type_id = value_type_id(args[idx]);

			if (param_type_id != arg_type_id)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Invalid type for argument %d (expected %d, was %d)", idx, param_type_id, arg_type_id);
				return NULL;
			}

			if (reflect_to_wasm_type(args[idx], &wasm_args[idx]) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Unsupported type for argument %d", idx);
				return NULL;
			}
		}

		const wasm_val_vec_t args_vec = WASM_ARRAY_VEC(wasm_args);

		return call_func(sig, wasm_func->func, args_vec);
	}
}

static void function_wasm_interface_destroy(function func, function_impl impl)
{
	(void)func;

	free((loader_impl_wasm_function)impl);
}
