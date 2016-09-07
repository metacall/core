/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* -- Definitions -- */

#define LOADER_LIBRARY_PATH "LOADER_LIBRARY_PATH"

#define LOADER_SCRIPT_PATH "LOADER_SCRIPT_PATH"

/* -- Forward Declarations -- */

struct loader_type;

struct loader_get_iterator_args_type;

/* -- Type Definitions -- */

typedef struct loader_type * loader;

typedef struct loader_get_iterator_args_type * loader_get_iterator_args;

/* -- Member Data -- */

struct loader_type
{
	hash_map impl_map;
	char * library_path;
	char * script_path;
};

struct loader_get_iterator_args_type
{
	const char * name;
	scope_object obj;

};

/* -- Methods -- */

loader loader_singleton(void)
{
	static struct loader_type loader_instance =
	{
		NULL, NULL, NULL
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

	if (l->library_path == NULL)
	{
		const char loader_library_path[] = LOADER_LIBRARY_PATH;

		l->library_path = getenv(loader_library_path);
	}

	if (l->script_path == NULL)
	{
		const char loader_script_path[] = LOADER_SCRIPT_PATH;

		l->script_path = getenv(loader_script_path);
	}
}

loader_impl loader_create_impl(loader_naming_extension extension)
{
	loader l = loader_singleton();

	loader_impl impl = loader_impl_create(l->library_path, extension);

	if (impl != NULL)
	{
		loader_naming_extension * extension_ptr = loader_impl_extension(impl);

		if (hash_map_insert(l->impl_map, *extension_ptr, impl) == 0)
		{
			if (loader_impl_execution_path(impl, ".") == 0)
			{
				if (l->library_path != NULL)
				{
					if (loader_impl_execution_path(impl, l->library_path) == 0)
					{
						return impl;
					}
				}
				else
				{
					return impl;
				}
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
				/*
				if (l->script_path != NULL)
				{
					loader_naming_path absolute_path;

					memcpy(absolute_path, l->script_path, strlen(l->script_path) + 1);

					strncat(absolute_path, path, LOADER_NAMING_PATH_SIZE);

					return loader_impl_load(impl, absolute_path);
				}
				else
				{
				*/
					return loader_impl_load(impl, path);
				//}
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

	if (l->library_path != NULL)
	{
		l->library_path = NULL;
	}

	if (l->script_path != NULL)
	{
		l->script_path = NULL;
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
