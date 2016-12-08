/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_RB_PORT_IMPL_I
#define METACALL_SWIG_WRAPPER_RB_PORT_IMPL_I 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Maps -- */

/**
*  @brief
*    Transform variadic arguments from Ruby into
*    a valid metacallv format with values
*/
%typemap(in) (const char * name, ...)
{
	value * args;
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

	/* TODO: Remove this by a local array? */
	args = (value *) malloc(args_size * sizeof(value));

	if (args == NULL)
	{
		rb_raise(rb_eArgError, "Invalid argument allocation");

		SWIG_fail;

		return Qnil;
	}

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		VALUE rb_arg = argv[args_count + 1];

		int rb_arg_type = TYPE(rb_arg);

		if (rb_arg_type == T_TRUE)
		{
			boolean b = 1L;

			args[args_count] = value_create_bool(b);
		}
		else if (rb_arg_type == T_FALSE)
		{
			boolean b = 0L;

			args[args_count] = value_create_bool(b);
		}
		else if (rb_arg_type == T_FIXNUM)
		{
			int i = FIX2INT(rb_arg);

			args[args_count] = value_create_int(i);
		}
		else if (rb_arg_type == T_BIGNUM)
		{
			long l = NUM2LONG(rb_arg);

			args[args_count] = value_create_long(l);
		}
		else if (rb_arg_type == T_FLOAT)
		{
			double d = NUM2DBL(rb_arg);

			args[args_count] = value_create_double(d);
		}
		else if (rb_arg_type == T_STRING)
		{
			long length = RSTRING_LEN(rb_arg);

			char * str = StringValuePtr(rb_arg);

			if (length > 0 && str != NULL)
			{
				args[args_count] = value_create_string(str, (size_t)length);
			}
		}
		else
		{
			/* TODO: Remove this by a local array? */
			free(args);

			rb_raise(rb_eArgError, "Invalid argument allocation");

			SWIG_fail;

			return Qnil;
		}
	}

	$2 = (void *) args;
}

/* -- Features -- */

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
	value * args, ret;

	args_size = argc - 1;
	args = (value *) arg2;

	/* Execute call */
	ret = metacallv(arg1, args);

	/* Clear args */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		value_destroy(args[args_count]);
	}

	/* TODO: Remove this by a local array? */
	free(args);

	/* Return value */
	if (ret != NULL)
	{
		switch (value_type_id(ret))
		{

			case TYPE_BOOL :
			{
				boolean b = value_to_bool(ret);

				/*$result*/ vresult = (b == 0L) ? Qfalse : Qtrue;

				break;
			}

			case TYPE_CHAR :
			{
				/*$result*/ vresult = INT2FIX((char)value_to_char(ret));

				break;
			}

			case TYPE_SHORT :
			{
				/*$result*/ vresult = INT2FIX((int)value_to_short(ret));

				break;
			}

			case TYPE_INT :
			{
				/*$result*/ vresult = INT2FIX(value_to_int(ret));

				break;
			}

			case TYPE_LONG :
			{
				/*$result*/ vresult = LONG2NUM(value_to_long(ret));

				break;
			}

			case TYPE_FLOAT :
			{
				/*$result*/ vresult = DBL2NUM((double)value_to_float(ret));

				break;
			}

			case TYPE_DOUBLE :
			{
				/*$result*/ vresult = DBL2NUM(value_to_double(ret));

				break;
			}

			case TYPE_STRING :
			{
				/*$result*/ vresult = rb_str_new_cstr(value_to_string(ret));

				break;
			}

			default :
			{
				rb_raise(rb_eArgError, "Unsupported return type");

				/*$result*/ vresult = Qnil;
			}
		}

		value_destroy(ret);
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
