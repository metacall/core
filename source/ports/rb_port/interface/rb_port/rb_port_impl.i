/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_SWIG_WRAPPER_RB_PORT_IMPL_I
#define METACALL_SWIG_WRAPPER_RB_PORT_IMPL_I 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Ignores -- */

%ignore metacall_null_args;

%ignore metacallv;

%ignore metacallvf;

%ignore metacall_serial; /* TODO */

%ignore metacall_register; /* TODO */

%ignore metacall_load_from_package; /* TODO */

/* -- Type Maps -- */

/**
*  @brief
*    Transform load mechanism from Ruby string into
*    a valid load from memory format (buffer and size)
*/
%typemap(in) (const char * buffer, size_t size, void ** handle)
{
	char * buffer_str = StringValuePtr($input);

	size_t length = RSTRING_LEN($input);

	$1 = buffer_str;

	$2 = (length + 1);
}

/**
*  @brief
*    Transform load mechanism from Ruby array into
*    a valid load from file format (array of strings)
*/
%typemap(in) (const char * paths[], size_t size, void ** handle)
{
	size_t iterator, size = RARRAY_LEN($input);

	VALUE * array_ptr = RARRAY_PTR($input);

	if (size == 0)
	{
		rb_raise(rb_eArgError, "Empty script path list");

		return Qnil;
	}

	$1 = (char **)malloc(sizeof(char *) * size);

	if ($1 == NULL)
	{
		rb_raise(rb_eArgError, "Invalid argument allocation");

		SWIG_fail;
	}

	$2 = size;

	for (iterator = 0; iterator < size; ++iterator, ++array_ptr)
	{
		size_t length = RSTRING_LEN(*array_ptr);

		$1[iterator] = malloc(sizeof(char) * (length + 1));

		if ($1[iterator] == NULL)
		{
			size_t alloc_iterator;

			for (alloc_iterator = 0; alloc_iterator < iterator; ++alloc_iterator)
			{
				free($1[alloc_iterator]);
			}

			free($1);

			rb_raise(rb_eArgError, "Invalid string path allocation");

			SWIG_fail;
		}

		memcpy($1[iterator], StringValuePtr(*array_ptr), length);

		$1[iterator][length] = '\0';
	}
}

/**
*  @brief
*    Transform variadic arguments from Ruby into
*    a valid metacallv format with values
*/
%typemap(in) (const char * name, ...)
{
	void ** args;
	size_t args_size, args_count;

	/* Format string */
	$1 = RSTRING_PTR($input);

	/* Variable length arguments */
	if (argc >= 1)
	{
		args_size = argc - 1;
	}
	else
	{
		rb_raise(rb_eArgError, "Invalid number of arguments");

		return Qnil;
	}

	if (args_size > 0)
	{
		/* TODO: Remove this by a local array? */
		args = (void **) malloc(args_size * sizeof(void *));

		if (args == NULL)
		{
			rb_raise(rb_eArgError, "Invalid argument allocation");

			SWIG_fail;
		}

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			VALUE rb_arg = argv[args_count + 1];

			int rb_arg_type = TYPE(rb_arg);

			if (rb_arg_type == T_TRUE)
			{
				boolean b = 1L;

				args[args_count] = metacall_value_create_bool(b);
			}
			else if (rb_arg_type == T_FALSE)
			{
				boolean b = 0L;

				args[args_count] = metacall_value_create_bool(b);
			}
			else if (rb_arg_type == T_FIXNUM)
			{
				int i = FIX2INT(rb_arg);

				args[args_count] = metacall_value_create_int(i);
			}
			else if (rb_arg_type == T_BIGNUM)
			{
				long l = NUM2LONG(rb_arg);

				args[args_count] = metacall_value_create_long(l);
			}
			else if (rb_arg_type == T_FLOAT)
			{
				double d = NUM2DBL(rb_arg);

				args[args_count] = metacall_value_create_double(d);
			}
			else if (rb_arg_type == T_STRING)
			{
				long length = RSTRING_LEN(rb_arg);

				char * str = StringValuePtr(rb_arg);

				if (length > 0 && str != NULL)
				{
					args[args_count] = metacall_value_create_string(str, (size_t)length);
				}
			}
			else if (rb_arg_type == T_NIL)
			{
				args[args_count] = metacall_value_create_null();
			}
			else
			{
				size_t alloc_iterator;

				for (alloc_iterator = 0; alloc_iterator < args_count; ++alloc_iterator)
				{
					metacall_value_destroy(args[alloc_iterator]);
				}

				/* TODO: Remove this by a local array? */
				free(args);

				rb_raise(rb_eArgError, "Invalid argument allocation");

				SWIG_fail;

				return Qnil;
			}
		}
	}
	else
	{
		args = NULL;
	}

	$2 = (void *) args;
}

/* -- Features -- */

/**
*  @brief
*    Execute the load from memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
%feature("action") metacall_load_from_memory
{
	const char * tag = (const char *)arg1;

	char * buffer = (char *)arg2;

	size_t size = (size_t)arg3;

	result = metacall_load_from_memory(tag, (const char *)buffer, size, NULL);
}

/**
*  @brief
*    Execute the load from file
*
*  @return
*    Zero if success, different from zero otherwise
*/
%feature("action") metacall_load_from_file
{
	const char * tag = (const char *)arg1;

	char ** paths = (char **)arg2;

	size_t iterator, size = arg3;

	result = metacall_load_from_file(tag, (const char **)paths, size, NULL);

	for (iterator = 0; iterator < size; ++iterator)
	{
		free(paths[iterator]);
	}

	free(paths);
}

/**
*  @brief
*    Execute the call and transform return
*    value into a valid Ruby format
*
*  @return
*    A value converted into Ruby format
*/
%feature("action") metacall
{
	size_t args_count, args_size;
	void ** args;
	void * ret;

	args_size = argc - 1;
	args = (void **) arg2;

	if (args != NULL)
	{
		/* Execute call */
		ret = metacallv(arg1, args);

		/* Clear args */
		for (args_count = 0; args_count < args_size; ++args_count)
		{
			metacall_value_destroy(args[args_count]);
		}

		/* TODO: Remove this by a local array? */
		free(args);
	}
	else
	{
		/* Execute call */
		ret = metacallv(arg1, metacall_null_args);
	}

	/* Return value */
	if (ret != NULL)
	{
		switch (metacall_value_id(ret))
		{

			case METACALL_BOOL :
			{
				boolean b = metacall_value_to_bool(ret);

				/*$result*/ vresult = (b == 0L) ? Qfalse : Qtrue;

				break;
			}

			case METACALL_CHAR :
			{
				/*$result*/ vresult = INT2FIX((char)metacall_value_to_char(ret));

				break;
			}

			case METACALL_SHORT :
			{
				/*$result*/ vresult = INT2FIX((int)metacall_value_to_short(ret));

				break;
			}

			case METACALL_INT :
			{
				/*$result*/ vresult = INT2FIX(metacall_value_to_int(ret));

				break;
			}

			case METACALL_LONG :
			{
				/*$result*/ vresult = LONG2NUM(metacall_value_to_long(ret));

				break;
			}

			case METACALL_FLOAT :
			{
				/*$result*/ vresult = DBL2NUM((double)metacall_value_to_float(ret));

				break;
			}

			case METACALL_DOUBLE :
			{
				/*$result*/ vresult = DBL2NUM(metacall_value_to_double(ret));

				break;
			}

			case METACALL_STRING :
			{
				/*$result*/ vresult = rb_str_new_cstr(metacall_value_to_string(ret));

				break;
			}

			case METACALL_NULL :
			{
				/*$result*/ vresult = Qnil;

				break;
			}

			default :
			{
				rb_raise(rb_eArgError, "Unsupported return type");

				/*$result*/ vresult = Qnil;
			}
		}

		metacall_value_destroy(ret);
	}
	else
	{
		/*$result*/ vresult = Qnil;
	}

	return /*$result*/ vresult;
}

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SWIG_WRAPPER_RB_PORT_IMPL_I */
