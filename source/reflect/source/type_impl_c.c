#include <reflect/type.h>
#include <reflect/type_impl_c.h>

type_impl type_create_impl_c(type t)
{
	return NULL;
}

type_instance type_instance_create_impl_c(type t)
{
	if (t != NULL)
	{
		size_t size = type_size(t);

		if (size > 0)
		{
			type_instance instance = malloc(size);

			if (instance != NULL)
			{
				return instance;
			}
		}
	}

	return NULL;
}

type_instance type_instance_crete_array_impl_c(type t, int count)
{
	if (t != NULL && count > 0)
	{
		size_t size = type_size(t);

		if (size > 0)
		{
			type_instance instance = malloc(size * count);

			if (instance != NULL)
			{
				return instance;
			}
		}
	}

	return NULL;
}

void type_instance_destroy_impl_c(type_instance instance)
{
	if (instance != NULL)
	{
		free(instance);
	}
}

void type_destroy_impl_c(type t, type_impl impl)
{

}

type_interface type_impl_c()
{
	static struct type_interface_type interface =
	{
		&type_create_impl_c,
		&type_instance_create_impl_c,
		&type_instance_create_array_impl_c,
		&type_instance_destroy_impl_c,
		&type_destroy_impl_c
	};

	return &interface;
}
