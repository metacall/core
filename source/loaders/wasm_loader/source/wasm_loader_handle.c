#include <wasm_loader/wasm_loader_function.h>
#include <wasm_loader/wasm_loader_handle.h>

#include <log/log.h>

struct loader_impl_wasm_module_type
{
	loader_name name;
	wasm_module_t *module;
	wasm_instance_t *instance;
	wasm_extern_vec_t exports;
	wasm_exporttype_vec_t export_types;
	wasm_extern_vec_t imports;
};

struct loader_impl_wasm_handle_type
{
	vector modules;
};

static int discover_module(loader_impl impl, scope scp, const loader_impl_wasm_module *module);
static int initialize_module_imports(loader_impl_wasm_handle handle, loader_impl_wasm_module *module);
static void delete_module(loader_impl_wasm_module *module);

loader_impl_wasm_handle wasm_loader_handle_create(size_t num_modules)
{
	loader_impl_wasm_handle handle = malloc(sizeof(struct loader_impl_wasm_handle_type));

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate memory for handle");
		goto error_handle_alloc;
	}

	handle->modules = vector_create_reserve_type(loader_impl_wasm_module, num_modules);

	if (handle->modules == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate memory for modules");
		goto error_modules_alloc;
	}

	return handle;

error_modules_alloc:
	free(handle);
error_handle_alloc:
	return NULL;
}

void wasm_loader_handle_destroy(loader_impl_wasm_handle handle)
{
	for (size_t idx = 0; idx < vector_size(handle->modules); idx++)
	{
		loader_impl_wasm_module *module = vector_at(handle->modules, idx);
		delete_module(module);
	}

	vector_destroy(handle->modules);
	free(handle);
}

int wasm_loader_handle_add_module(loader_impl_wasm_handle handle, const loader_name name, wasm_store_t *store, const wasm_byte_vec_t *binary)
{
	loader_impl_wasm_module module;
	module.module = wasm_module_new(store, binary);

	if (module.module == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to create module");
		goto error_module_new;
	}

	if (initialize_module_imports(handle, &module) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Could not satisfy all imports required by module");
		goto error_initialize_imports;
	}

	strncpy(module.name, name, LOADER_NAME_SIZE - 1);

	// There is no way to check whether `wasm_module_exports` or
	// `wasm_instance_new` fail, so just hope for the best.
	wasm_module_exports(module.module, &module.export_types);

	module.instance = wasm_instance_new(store, module.module, &module.imports, NULL);
	wasm_instance_exports(module.instance, &module.exports);

	vector_push_back_var(handle->modules, module);

	return 0;

error_initialize_imports:
	wasm_module_delete(module.module);
error_module_new:
	return 1;
}

int wasm_loader_handle_discover(loader_impl impl, loader_impl_wasm_handle handle, scope scp)
{
	for (size_t idx = 0; idx < vector_size(handle->modules); idx++)
	{
		if (discover_module(impl, scp, vector_at(handle->modules, idx)) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Handle discovery failed");
			return 1;
		}
	}

	return 0;
}

static bool is_same_valtype(const wasm_valtype_t *a, const wasm_valtype_t *b)
{
	return wasm_valtype_kind(a) == wasm_valtype_kind(b);
}

static bool is_same_valtype_vec(const wasm_valtype_vec_t *a, const wasm_valtype_vec_t *b)
{
	if (a->size != b->size)
	{
		return false;
	}

	for (size_t i = 0; i < a->size; i++)
	{
		if (!is_same_valtype(a->data[i], b->data[i]))
		{
			return false;
		}
	}

	return true;
}

static type valkind_to_type(loader_impl impl, wasm_valkind_t kind)
{
	switch (kind)
	{
		case WASM_I32:
			return loader_impl_type(impl, "i32");
		case WASM_I64:
			return loader_impl_type(impl, "i64");
		case WASM_F32:
			return loader_impl_type(impl, "f32");
		case WASM_F64:
			return loader_impl_type(impl, "f64");
		default:
			return NULL;
	}
}

static char *get_export_type_name(const wasm_exporttype_t *export_type)
{
	const wasm_name_t *name = wasm_exporttype_name(export_type);
	char *null_terminated_name = malloc(name->size + 1);

	if (null_terminated_name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate memory for name of export type");
		return NULL;
	}

	strncpy(null_terminated_name, name->data, name->size);
	null_terminated_name[name->size] = '\0';

	return null_terminated_name;
}

static int discover_function(loader_impl impl, scope scp, const wasm_externtype_t *extern_type, const char *name, const wasm_extern_t *extern_val)
{
	if (scope_get(scp, name) != NULL)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "WebAssembly loader: A function named \"%s\" is already defined, skipping redefinition", name);
		return 0;
	}

	const wasm_functype_t *func_type = wasm_externtype_as_functype_const(extern_type);
	const wasm_valtype_vec_t *params = wasm_functype_params(func_type);
	const wasm_valtype_vec_t *results = wasm_functype_results(func_type);

	loader_impl_wasm_function func_impl = loader_impl_wasm_function_create(wasm_extern_as_func_const(extern_val), results->size);

	if (func_impl == NULL)
	{
		return 1;
	}

	function func = function_create(name, params->size, func_impl, &function_wasm_singleton);
	signature sig = function_signature(func);

	if (results->size > 0)
	{
		type ret_type = results->size == 1 ? valkind_to_type(impl, wasm_valtype_kind(results->data[0])) : loader_impl_type(impl, "array");
		signature_set_return(sig, ret_type);
	}

	for (size_t param_idx = 0; param_idx < params->size; param_idx++)
	{
		signature_set(sig, param_idx, "unnamed", valkind_to_type(impl, wasm_valtype_kind(params->data[param_idx])));
	}

	value v = value_create_function(func);

	if (scope_define(scp, function_name(func), v) != 0)
	{
		value_type_destroy(v);
		return 1;
	}

	return 0;
}

static int discover_export(loader_impl impl, scope scp, const wasm_exporttype_t *export_type, const wasm_extern_t *export)
{
	int ret = 1;

	// All of the `wasm_*` calls here return pointers to memory owned by
	// `export_types`, so no need to do any error checking or cleanup.
	const wasm_externtype_t *extern_type = wasm_exporttype_type(export_type);
	const wasm_externkind_t kind = wasm_externtype_kind(extern_type);
	char *export_name = get_export_type_name(export_type);

	if (export_name == NULL)
	{
		goto error_export_name_alloc;
	}

	if (kind == WASM_EXTERN_FUNC)
	{
		if (discover_function(impl, scp, extern_type, export_name, export) != 0)
		{
			goto error_discover_function;
		}
	}

	ret = 0;

error_discover_function:
	free(export_name);
error_export_name_alloc:
	return ret;
}

static int discover_module(loader_impl impl, scope scp, const loader_impl_wasm_module *module)
{
	for (size_t i = 0; i < module->export_types.size; i++)
	{
		// There is a 1-to-1 correspondence between between the instance
		// exports and the module exports, so we can use the same index.
		const wasm_exporttype_t *export_type = module->export_types.data[i];
		const wasm_extern_t *export = module->exports.data[i];
		if (discover_export(impl, scp, export_type, export) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Module discovery failed");
			return 1;
		}
	}

	return 0;
}

static bool is_suitable_limit(const wasm_limits_t *import, const wasm_limits_t *export)
{
	return import->min <= export->min && import->max <= export->max;
}

static bool matches_memory_import(const wasm_memorytype_t *import, const wasm_memorytype_t *export)
{
	return is_suitable_limit(wasm_memorytype_limits(import), wasm_memorytype_limits(export));
}

static bool matches_func_import(const wasm_functype_t *import, const wasm_functype_t *export)
{
	return is_same_valtype_vec(wasm_functype_params(import), wasm_functype_params(export)) &&
		   is_same_valtype_vec(wasm_functype_results(import), wasm_functype_results(export));
}

static bool matches_table_import(const wasm_tabletype_t *import, const wasm_tabletype_t *export)
{
	return is_same_valtype(wasm_tabletype_element(import), wasm_tabletype_element(export)) && is_suitable_limit(wasm_tabletype_limits(import), wasm_tabletype_limits(export));
}

static bool matches_global_import(const wasm_globaltype_t *import, const wasm_globaltype_t *export)
{
	return is_same_valtype(wasm_globaltype_content(import), wasm_globaltype_content(export)) && wasm_globaltype_mutability(import) == wasm_globaltype_mutability(export);
}

static bool matches_import(const wasm_externtype_t *import, const wasm_externtype_t *export)
{
	const wasm_externkind_t import_kind = wasm_externtype_kind(import);
	const wasm_externkind_t export_kind = wasm_externtype_kind(export);

	if (import_kind != export_kind)
	{
		return false;
	}

	if (import_kind == WASM_EXTERN_FUNC)
	{
		return matches_func_import(wasm_externtype_as_functype_const(import), wasm_externtype_as_functype_const(export));
	}
	else if (import_kind == WASM_EXTERN_MEMORY)
	{
		return matches_memory_import(wasm_externtype_as_memorytype_const(import), wasm_externtype_as_memorytype_const(export));
	}
	else if (import_kind == WASM_EXTERN_TABLE)
	{
		return matches_table_import(wasm_externtype_as_tabletype_const(import), wasm_externtype_as_tabletype_const(export));
	}
	else if (import_kind == WASM_EXTERN_GLOBAL)
	{
		return matches_global_import(wasm_externtype_as_globaltype_const(import), wasm_externtype_as_globaltype_const(export));
	}
	else
	{
		return false;
	}
}

static const wasm_extern_t *find_export(loader_impl_wasm_handle handle, const wasm_importtype_t *import_type)
{
	const wasm_name_t *module_name = wasm_importtype_module(import_type);
	const wasm_name_t *name = wasm_importtype_name(import_type);
	const wasm_externtype_t *type = wasm_importtype_type(import_type);

	for (size_t module_idx = 0; module_idx < vector_size(handle->modules); module_idx++)
	{
		loader_impl_wasm_module *module = vector_at(handle->modules, module_idx);

		// module->name is null-terminated, so no need to compare sizes first
		if (strncmp(module->name, module_name->data, module_name->size) != 0)
		{
			continue;
		}

		for (size_t export_idx = 0; export_idx < module->exports.size; export_idx++)
		{
			const wasm_name_t *export_name = wasm_exporttype_name(module->export_types.data[export_idx]);
			wasm_externtype_t *export_type = wasm_extern_type(module->exports.data[export_idx]);

			if (export_name->size == name->size && strncmp(export_name->data, name->data, name->size) == 0 &&
				matches_import(type, export_type))
			{
				wasm_externtype_delete(export_type);
				return module->exports.data[export_idx];
			}

			wasm_externtype_delete(export_type);
		}
	}

	return NULL;
}

static int initialize_module_imports(loader_impl_wasm_handle handle, loader_impl_wasm_module *module)
{
	wasm_importtype_vec_t import_types;
	wasm_module_imports(module->module, &import_types);
	wasm_extern_vec_new_uninitialized(&module->imports, import_types.size);

	for (size_t i = 0; i < import_types.size; i++)
	{
		const wasm_extern_t *import = find_export(handle, import_types.data[i]);

		if (import == NULL)
		{
			goto error_find_import;
		}

		module->imports.data[i] = wasm_extern_copy(import);
	}

	wasm_importtype_vec_delete(&import_types);
	return 0;

error_find_import:
	wasm_extern_vec_delete(&module->imports);
	wasm_importtype_vec_delete(&import_types);
	return 1;
}

static void delete_module(loader_impl_wasm_module *module)
{
	wasm_exporttype_vec_delete(&module->export_types);
	wasm_extern_vec_delete(&module->exports);
	wasm_instance_delete(module->instance);
	wasm_extern_vec_delete(&module->imports);
	wasm_module_delete(module->module);
}
