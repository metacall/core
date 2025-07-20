/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

/* -- Headers -- */

#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <stdint.h>

/* -- Methods -- */

value value_type_create(const void *data, size_t bytes, type_id id)
{
	value v = value_alloc(bytes + sizeof(type_id));

	if (v == NULL)
	{
		return NULL;
	}

	/* Memset body */
	value_from(v, data, bytes);

	/* Memset header */
	value_from((value)(((uintptr_t)v) + bytes), &id, sizeof(type_id));

	return v;
}

value value_type_copy(value v)
{
	if (v != NULL)
	{
		type_id id = value_type_id(v);

		if (type_id_array(id) == 0)
		{
			size_t index, size = value_type_count(v);

			value new_v = value_create_array(NULL, size);

			value *new_v_array = value_to_array(new_v);

			value *v_array = value_to_array(v);

			for (index = 0; index < size; ++index)
			{
				new_v_array[index] = value_type_copy(v_array[index]);
			}

			return new_v;
		}
		else if (type_id_map(id) == 0)
		{
			size_t index, size = value_type_count(v);

			value new_v = value_create_map(NULL, size);

			value *new_v_map = value_to_map(new_v);

			value *v_map = value_to_map(v);

			for (index = 0; index < size; ++index)
			{
				new_v_map[index] = value_type_copy(v_map[index]);
			}

			return new_v;
		}
		else if (type_id_function(id) == 0)
		{
			value cpy = value_copy(v);

			if (cpy != NULL)
			{
				function f = value_to_function(cpy);

				function_increment_reference(f);
			}

			return cpy;
		}
		else if (type_id_class(id) == 0)
		{
			value cpy = value_copy(v);

			if (cpy != NULL)
			{
				klass cls = value_to_class(v);

				class_increment_reference(cls);
			}

			return cpy;
		}
		else if (type_id_object(id) == 0)
		{
			value cpy = value_copy(v);

			if (cpy != NULL)
			{
				object obj = value_to_object(cpy);

				object_increment_reference(obj);
			}

			return cpy;
		}
		else if (type_id_exception(id) == 0)
		{
			value cpy = value_copy(v);

			if (cpy != NULL)
			{
				exception ex = value_to_exception(cpy);

				exception_increment_reference(ex);
			}

			return cpy;
		}
		else if (type_id_throwable(id) == 0)
		{
			/* Just create a new throwable from the previous one, it will get flattened after creation */
			return value_create_throwable(v);
		}

		if (type_id_invalid(id) != 0)
		{
			return value_copy(v);
		}
	}

	return NULL;
}

value value_type_reference(value v)
{
	void *data = value_data(v);

	return value_create_ptr(data);
}

value value_type_dereference(value v)
{
	void *data = value_to_ptr(v);

	return value_container(data);
}

size_t value_type_size(value v)
{
	size_t size = value_size(v);

	return size - sizeof(type_id);
}

size_t value_type_count(void *v)
{
	const type_id id = value_type_id(v);

	if (id == TYPE_ARRAY || id == TYPE_MAP)
	{
		/* Array and map can contain multiple values */
		return value_type_size(v) / sizeof(const value);
	}
	else if (id == TYPE_INVALID)
	{
		/* Invalid does not contain any value */
		return 0;
	}

	/* Rest of values only can contain one value */
	return 1;
}

type_id value_type_id(value v)
{
	type_id id = TYPE_INVALID;

	if (v != NULL)
	{
		size_t size = value_size(v);

		size_t offset = size - sizeof(type_id);

		value_to((value)(((uintptr_t)v) + offset), &id, sizeof(type_id));
	}

	return id;
}

value value_create_bool(boolean b)
{
	return value_type_create(&b, sizeof(boolean), TYPE_BOOL);
}

value value_create_char(char c)
{
	return value_type_create(&c, sizeof(char), TYPE_CHAR);
}

value value_create_short(short s)
{
	return value_type_create(&s, sizeof(short), TYPE_SHORT);
}

value value_create_int(int i)
{
	return value_type_create(&i, sizeof(int), TYPE_INT);
}

value value_create_long(long l)
{
	return value_type_create(&l, sizeof(long), TYPE_LONG);
}

value value_create_float(float f)
{
	return value_type_create(&f, sizeof(float), TYPE_FLOAT);
}

value value_create_double(double d)
{
	return value_type_create(&d, sizeof(double), TYPE_DOUBLE);
}

value value_create_string(const char *str, size_t length)
{
	return value_type_create(str, sizeof(char) * (length + 1), TYPE_STRING);
}

value value_create_buffer(const void *buffer, size_t size)
{
	return value_type_create(buffer, sizeof(char) * size, TYPE_BUFFER);
}

value value_create_array(const value *values, size_t size)
{
	return value_type_create(values, sizeof(const value) * size, TYPE_ARRAY);
}

value value_create_map(const value *tuples, size_t size)
{
	return value_type_create(tuples, sizeof(const value) * size, TYPE_MAP);
}

value value_create_ptr(const void *ptr)
{
	return value_type_create(&ptr, sizeof(const void *), TYPE_PTR);
}

value value_create_future(future f)
{
	return value_type_create(&f, sizeof(future), TYPE_FUTURE);
}

value value_create_function(function f)
{
	value v = value_type_create(&f, sizeof(function), TYPE_FUNCTION);

	if (v != NULL)
	{
		function_increment_reference(f);
	}

	return v;
}

value value_create_function_closure(function f, void *c)
{
	// TODO: Review this for the lock-free implementation!!
	// The functions should be immutable, maybe the binding should be a new type
	// or the bind should be stored in the value instead of in the function

	value v = value_type_create(&f, sizeof(function), TYPE_FUNCTION);

	if (v != NULL)
	{
		function_bind(f, c);
		function_increment_reference(f);
	}

	return v;
}

value value_create_null(void)
{
	return value_type_create(NULL, 0, TYPE_NULL);
}

value value_create_class(klass c)
{
	value v = value_type_create(&c, sizeof(klass), TYPE_CLASS);

	if (v != NULL)
	{
		class_increment_reference(c);
	}

	return v;
}

value value_create_object(object o)
{
	value v = value_type_create(&o, sizeof(object), TYPE_OBJECT);

	if (v != NULL)
	{
		object_increment_reference(o);
	}

	return v;
}

value value_create_exception(exception ex)
{
	value v = value_type_create(&ex, sizeof(exception), TYPE_EXCEPTION);

	if (v != NULL)
	{
		exception_increment_reference(ex);
	}

	return v;
}

value value_create_throwable(throwable th)
{
	return value_type_create(&th, sizeof(throwable), TYPE_THROWABLE);
}

boolean value_to_bool(value v)
{
	boolean b = 0;

	value_to(v, &b, sizeof(boolean));

	return b;
}

char value_to_char(value v)
{
	char c = '\0';

	value_to(v, &c, sizeof(char));

	return c;
}

short value_to_short(value v)
{
	short s = 0;

	value_to(v, &s, sizeof(short));

	return s;
}

int value_to_int(value v)
{
	int i = 0;

	value_to(v, &i, sizeof(int));

	return i;
}

long value_to_long(value v)
{
	long l = 0L;

	value_to(v, &l, sizeof(long));

	return l;
}

float value_to_float(value v)
{
	float f = 0.0f;

	value_to(v, &f, sizeof(float));

	return f;
}

double value_to_double(value v)
{
	double d = 0.0;

	value_to(v, &d, sizeof(double));

	return d;
}

char *value_to_string(value v)
{
	return value_data(v);
}

void *value_to_buffer(value v)
{
	return value_data(v);
}

value *value_to_array(value v)
{
	return value_data(v);
}

value *value_to_map(value v)
{
	return value_data(v);
}

void *value_to_ptr(value v)
{
	uintptr_t *uint_ptr = value_data(v);

	return (void *)(*uint_ptr);
}

future value_to_future(value v)
{
	uintptr_t *uint_future = value_data(v);

	return (future)(*uint_future);
}

function value_to_function(value v)
{
	uintptr_t *uint_function = value_data(v);

	return (function)(*uint_function);
}

void *value_to_null(value v)
{
	(void)v;

	return (void *)NULL;
}

klass value_to_class(value v)
{
	uintptr_t *uint_class = value_data(v);

	return (klass)(*uint_class);
}

object value_to_object(value v)
{
	uintptr_t *uint_object = value_data(v);

	return (object)(*uint_object);
}

exception value_to_exception(value v)
{
	uintptr_t *uint_exception = value_data(v);

	return (exception)(*uint_exception);
}

throwable value_to_throwable(value v)
{
	uintptr_t *uint_throwable = value_data(v);

	return (throwable)(*uint_throwable);
}

value value_from_bool(value v, boolean b)
{
	return value_from(v, &b, sizeof(boolean));
}

value value_from_char(value v, char c)
{
	return value_from(v, &c, sizeof(char));
}

value value_from_short(value v, short s)
{
	return value_from(v, &s, sizeof(short));
}

value value_from_int(value v, int i)
{
	return value_from(v, &i, sizeof(int));
}

value value_from_long(value v, long l)
{
	return value_from(v, &l, sizeof(long));
}

value value_from_float(value v, float f)
{
	return value_from(v, &f, sizeof(float));
}

value value_from_double(value v, double d)
{
	return value_from(v, &d, sizeof(double));
}

value value_from_string(value v, const char *str, size_t length)
{
	if (v != NULL)
	{
		if (str == NULL || length == 0)
		{
			return value_from(v, NULL, 1);
		}
		else
		{
			size_t current_size = value_type_size(v);

			size_t bytes = length + 1;

			size_t size = (bytes <= current_size) ? bytes : current_size;

			value_from(v, str, size);

			if (bytes > current_size)
			{
				char *str = value_to_string(v);

				str[size - 1] = '\0';
			}
		}
	}

	return v;
}

value value_from_buffer(value v, const void *buffer, size_t size)
{
	if (v != NULL && buffer != NULL && size > 0)
	{
		size_t current_size = value_type_size(v);

		size_t bytes = sizeof(char) * size;

		return value_from(v, buffer, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_array(value v, const value *values, size_t size)
{
	if (v != NULL && values != NULL && size > 0)
	{
		size_t current_size = value_type_size(v);

		size_t bytes = sizeof(const value) * size;

		return value_from(v, values, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_map(value v, const value *tuples, size_t size)
{
	if (v != NULL && tuples != NULL && size > 0)
	{
		size_t current_size = value_type_size(v);

		size_t bytes = sizeof(const value) * size;

		return value_from(v, tuples, (bytes <= current_size) ? bytes : current_size);
	}

	return v;
}

value value_from_ptr(value v, const void *ptr)
{
	return value_from(v, &ptr, sizeof(const void *));
}

value value_from_future(value v, future f)
{
	return value_from(v, &f, sizeof(future));
}

value value_from_function(value v, function f)
{
	return value_from(v, &f, sizeof(function));
}

value value_from_null(value v)
{
	return value_from(v, NULL, 0);
}

value value_from_class(value v, klass c)
{
	return value_from(v, &c, sizeof(klass));
}

value value_from_object(value v, object o)
{
	return value_from(v, &o, sizeof(object));
}

value value_from_exception(value v, exception ex)
{
	return value_from(v, &ex, sizeof(exception));
}

value value_from_throwable(value v, throwable th)
{
	return value_from(v, &th, sizeof(throwable));
}

void value_type_destroy(value v)
{
	/* TODO: Disable logs here until log is completely thread safe and async signal safe */

	if (v != NULL)
	{
		type_id id = value_type_id(v);

		if (type_id_array(id) == 0)
		{
			size_t index, size = value_type_count(v);

			value *v_array = value_to_array(v);

			/* log_write("metacall", LOG_LEVEL_DEBUG, "Destroy array value <%p> of size %u", (void *)v, size); */

			for (index = 0; index < size; ++index)
			{
				value_type_destroy(v_array[index]);
			}
		}
		else if (type_id_map(id) == 0)
		{
			size_t index, size = value_type_count(v);

			value *v_map = value_to_map(v);

			/* log_write("metacall", LOG_LEVEL_DEBUG, "Destroy map value <%p> of size %u", (void *)v, size); */

			for (index = 0; index < size; ++index)
			{
				value_type_destroy(v_map[index]);
			}
		}
		else if (type_id_future(id) == 0)
		{
			future f = value_to_future(v);

			/* log_write("metacall", LOG_LEVEL_DEBUG, "Destroy future value <%p>", (void *)v); */

			future_destroy(f);
		}
		else if (type_id_function(id) == 0)
		{
			function f = value_to_function(v);

			/*
			const char *name = function_name(f);

			if (name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous function <%p> value <%p>", (void *)f, (void *)v);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy function %s <%p> value <%p>", name, (void *)f, (void *)v);
			}
			*/

			function_destroy(f);
		}
		else if (type_id_class(id) == 0)
		{
			klass c = value_to_class(v);

			/*
			const char *name = class_name(c);

			if (name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous class <%p> value <%p>", (void *)c, (void *)v);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy class %s <%p> value <%p>", name, (void *)c, (void *)v);
			}
			*/

			class_destroy(c);
		}
		else if (type_id_object(id) == 0)
		{
			object o = value_to_object(v);
			int delete_return;

			/*
			const char *name = object_name(o);

			if (name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous object <%p> value <%p>", (void *)o, (void *)v);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy object %s <%p> value <%p>", name, (void *)o, (void *)v);
			}
			*/

			delete_return = object_delete(o);

			if (delete_return != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid deletion of object <%p>, destructor return error code %d", (void *)o, delete_return);
			}

			object_destroy(o);
		}
		else if (type_id_exception(id) == 0)
		{
			exception ex = value_to_exception(v);

			/* log_write("metacall", LOG_LEVEL_DEBUG, "Destroy exception value <%p>", (void *)v); */

			exception_destroy(ex);
		}
		else if (type_id_throwable(id) == 0)
		{
			throwable th = value_to_throwable(v);

			/* log_write("metacall", LOG_LEVEL_DEBUG, "Destroy throwable value <%p> containing the value <%p>", (void *)v, (void *)throwable_value(th)); */

			throwable_destroy(th);
		}

		if (type_id_invalid(id) != 0)
		{
			value_destroy(v);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Trying to destroy an invalid value <%p>", (void *)v);
		}
	}
}
