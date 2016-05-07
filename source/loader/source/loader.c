/************************************************************************/
/*	Loader Library by Parra Studios										*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for loading executable code at run-time into a process.	*/
/*																		*/
/************************************************************************/

#include <metacall/metacall-version.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <stdio.h>

typedef struct loader_type
{
	hash_map impl_map;

} * loader;

loader loader_singleton(void)
{
	static struct loader_type loader_instance =
	{
		NULL
	};

	return &loader_instance;
}

void loader_initialize(void)
{
	loader l = loader_singleton();

	if (l->impl_map == NULL)
	{
		l->impl_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);
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
			return impl;
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
	}

	return impl;
}

int loader_load(loader_naming_name name)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		loader_naming_extension extension;

		if (loader_naming_get_extension(name, extension) == 0)
		{
			loader_impl impl = loader_get_impl(extension);

			if (impl != NULL)
			{
				return loader_impl_load(impl, name);
			}
		}
	}

	return 1;
}

int loader_load_path(loader_naming_path path)
{
	loader l = loader_singleton();

	#ifdef LOADER_LAZY
		loader_initialize();
	#endif

	if (l->impl_map != NULL)
	{
		(void)path;

		/* ... */
	}

	return 1;
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
				loader_destroy();
			#endif

			return 1;
		}
	}

	#ifdef LOADER_LAZY
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
