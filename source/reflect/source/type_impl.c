/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/type.h>
#include <reflect/type_impl.h>

type_impl type_create_impl_null(type t)
{
	(void)t;

	return NULL;
}

type_instance type_instance_create_impl_null(type t)
{
	(void)t;

	return NULL;
}

type_instance type_instance_create_array_impl_null(type t, int count)
{
	(void)t; (void)count;

	return NULL;
}

void type_instance_destroy_impl_null(type t, type_instance instance)
{
	(void)t; (void)instance;
}

void type_destroy_impl_null(type t, type_impl impl)
{
	(void)t; (void)impl;
}

type_interface type_interface_impl_null(void)
{
	static struct type_interface_type interface =
	{
		&type_create_impl_null,
		&type_instance_create_impl_null,
		&type_instance_create_array_impl_null,
		&type_instance_destroy_impl_null,
		&type_destroy_impl_null
	};

	return &interface;
}
