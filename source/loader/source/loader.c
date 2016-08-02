/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <metacall/metacall-version.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>

#include <stdio.h>

typedef struct loader_type
{
	hash_map impl_map;

} * loader;

typedef struct loader_get_iterator_args_type
{
	const char * name;
	scope_object obj;

} * loader_get_iterator_args;

loader loader_singleton(void)
{
	static struct loader_type loader_instance =
	{
		NULL
	};

	return &loader_instance;
}

void loader_initialize()
{
	loader l = loader_singleton();

	if (l->impl_map == NULL)
	{
		l->impl_map = hash_map_create(&hash_callback_str, &comparable_callback_str);
	}
}

loader_impl loader_create_impl(loader_naming_extension extension)
{
	loader l = loader_singleton();

	loader_impl impl = loader_impl_create(extension);

	if (impl != NULL)
	{
		loader_naming_extension * extension_ptr = loader_impl_extension(impl);

		if (hash_map_insert(l->impl_map, *extension_ptr, impl) == 0)
		{
			if (loader_impl_execution_path(impl, ".") == 0)
			{
				return impl;
			}

			hash_map_remove(l->impl_map, *extension_ptr);
		}

		loader_impl_destroy(impl);
	}

	return NULL;
}

loader_impl loader_get_impl(loader_naming_extension extension)
{
	loader l = loader_singleton();

	loader_impl impl = (loader_impl)hash_map_get(l->impl_map, extension);

	if (impl == NULL)
	{
		impl = loader_create_impl(extension);

		printf("Create loader implementation: %s %p\n", extension, (void *)impl);
	}

	return impl;
}

int loader_load(const loader_naming_path path)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		printf("Loader lazy initialization\n");

		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		loader_naming_extension extension;

		if (loader_path_get_extension(path, extension) > 1)
		{
			loader_impl impl = loader_get_impl(extension);

			printf("Loader implementation (%s): %p\n", extension, (void *)impl);

			if (impl != NULL)
			{
				return loader_impl_load(impl, path);
			}
		}
	}

	return 1;
}

int loader_load_path(const loader_naming_path path)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		printf("Loader lazy initialization\n");

		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		(void)path;

		/* ... */
	}

	return 1;
}

int loader_get_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && value != NULL && args != NULL)
	{
		loader_impl impl = value;

		loader_get_iterator_args get_args = args;

		context ctx = loader_impl_context(impl);

		scope sp = context_scope(ctx);

		get_args->obj = scope_get(sp, get_args->name);

		if (get_args->obj != NULL)
		{
			printf("Loader get callback: impl %p, name %s\n", (void *)get_args->obj, get_args->name);

			return 1;
		}
	}

	return 0;
}

loader_data loader_get(const char * name)
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		hash_map_cb_iterate_args args;

		struct loader_get_iterator_args_type get_args;

		get_args.name = name;
		get_args.obj = NULL;

		args = (loader_get_iterator_args)&get_args;

		hash_map_iterate(l->impl_map, &loader_get_cb_iterate, args);

		if (get_args.obj != NULL)
		{
			return (loader_data)get_args.obj;
		}
	}

	return NULL;
}

int loader_unload_impl_map_cb_iterate(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && value != NULL && args == NULL)
	{
		loader_impl impl = value;

		loader_impl_destroy(impl);

		return 0;
	}

	return 1;
}

int loader_unload()
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		hash_map_iterate(l->impl_map, &loader_unload_impl_map_cb_iterate, NULL);

		if (hash_map_clear(l->impl_map) != 0)
		{
			#ifdef LOADER_LAZY
				printf("Loader lazy destruction\n");

				loader_destroy();
			#endif

			return 1;
		}
	}

	#ifdef LOADER_LAZY
		printf("Loader lazy destruction\n");

		loader_destroy();
	#endif

	return 0;
}

void loader_destroy()
{
	loader l = loader_singleton();

	if (l->impl_map != NULL)
	{
		hash_map_destroy(l->impl_map);

		l->impl_map = NULL;
	}
}

void loader_print_info()
{
	printf("Loader Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef LOADER_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif

	#ifdef LOADER_LAZY
		printf("Compiled with lazy initialization and destruction\n");
	#else
		printf("Compiled with explicit initialization and destruction\n");
	#endif
}
