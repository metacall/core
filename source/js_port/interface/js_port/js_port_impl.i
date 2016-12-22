/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 */

#ifndef METACALL_SWIG_WRAPPER_JS_PORT_IMPL_I
#define METACALL_SWIG_WRAPPER_JS_PORT_IMPL_I 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Maps -- */

/**
*  @brief
*    Transform variadic arguments from JavaScript into
*    a valid metacallv format with values
*/
%typemap(in) (const char * name, ...)(void * vargs[16], goto jmp_args_check)
{

/* Go to trick in order to avoid argument size checking */
jmp_args_check:

	/*value * vargs;*/
	size_t args_size, args_count;

	/* Format string */
	String::Utf8Value str_name($input);

	$1 = *str_name;

	/* Variable length arguments */
	args_size = args.Length();

	if (args_size == 0)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Invalid number of arguments",
			NewStringType::kNormal).ToLocalChecked());

		return;
	}
	else
	{
		/* Remove first argument */
		--args_size;
	}

	/* TODO: Remove this by a local array? */
	/*vargs = (value *) malloc(args_size * sizeof(value));*/

	if (vargs == NULL)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Invalid argument allocation",
			NewStringType::kNormal).ToLocalChecked());

		SWIG_fail;

		return;
	}

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		Local<Value> js_arg = args[args_count + 1];

		if (js_arg->IsBoolean())
		{
			boolean b = (js_arg->BooleanValue() == true) ? 1L : 0L;

 			vargs[args_count] = value_create_bool(b);
		}
		else if (js_arg->IsInt32())
		{
			/* Assume int is at least 32-bit width */
			int i = (int)js_arg->Int32Value();

			vargs[args_count] = value_create_int(i);
		}
		/*else if (js_arg->IsInteger())
		{
			*//* Assume long is at least 64-bit width *//*
			long l = (long)js_arg->IntegerValue();

			vargs[args_count] = value_create_long(l);
		}*/
		else if (js_arg->IsNumber())
		{
			double d = js_arg->NumberValue();

			vargs[args_count] = value_create_double(d);
		}
		else if (js_arg->IsString())
		{
			String::Utf8Value str(js_arg->ToString(args.GetIsolate()));

			vargs[args_count] = value_create_string(*str, str.length());
		}
		else if (js_arg->IsNull() || js_arg->IsUndefined())
		{
			vargs[args_count] = NULL;
		}
		else
		{
			/* TODO: Remove this by a local array? */
			/*free(vargs);*/

			args.GetIsolate()->ThrowException(
				String::NewFromUtf8(args.GetIsolate(), "Unsupported argument type",
				NewStringType::kNormal).ToLocalChecked());

			SWIG_fail;

			return;
		}
	}

	$2 = (void *) vargs;
}

/* -- Features -- */

/**
*  @brief
*    Execute the call and transform return
*    value into a valid JavaScript format
*
*  @return
*    A value converted into JavaScript format
*/
%feature("action") metacall
{
	size_t args_count, args_size;
	value * vargs, ret;

	args_size = args.Length() - 1;
	vargs = (value *) arg2;

	/* Execute call */
	ret = metacallv(arg1, vargs);

	/* Clear args */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		value_destroy(vargs[args_count]);
	}

	/* TODO: Remove this by a local array? */
	/*free(vargs);*/

	/* Return value */
	if (ret != NULL)
	{
		switch (value_type_id(ret))
		{

			case TYPE_BOOL :
			{
				bool b = ((long)value_to_bool(ret) == 1L ? true : false);

				$result = Boolean::New(args.GetIsolate(), b);

				break;
			}

			case TYPE_CHAR :
			{
				char c = value_to_char(ret);

				$result = String::NewFromUtf8(args.GetIsolate(), &c, String::kNormalString, 1);

				break;
			}

			case TYPE_SHORT :
			{
				short s = value_to_short(ret);

				$result = Integer::New(args.GetIsolate(), (int32_t)s);

				break;
			}

			case TYPE_INT :
			{
				int i = value_to_int(ret);

				$result = Integer::New(args.GetIsolate(), (int32_t)i);

				break;
			}

			case TYPE_LONG :
			{
				long l = value_to_long(ret);

				/* TODO: Check cast... */
				$result = Integer::New(args.GetIsolate(), (int32_t)l);

				break;
			}

			case TYPE_FLOAT :
			{
				float f = value_to_float(ret);

				$result = Number::New(args.GetIsolate(), (double)f);

				break;
			}

			case TYPE_DOUBLE :
			{
				double d = value_to_double(ret);

				$result = Number::New(args.GetIsolate(), d);

				break;
			}

			case TYPE_STRING :
			{
				const char * str = value_to_string(ret);

				$result = String::NewFromUtf8(args.GetIsolate(), str);

				break;
			}

			default :
			{
				args.GetIsolate()->ThrowException(
					String::NewFromUtf8(args.GetIsolate(), "Unsupported return type",
					NewStringType::kNormal).ToLocalChecked());

				$result = Null(args.GetIsolate());
			}
		}

		value_destroy(ret);
	}
	else
	{
		$result = Null(args.GetIsolate());
	}

	SWIGV8_RETURN($result);
}

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SWIG_WRAPPER_JS_PORT_IMPL_I */
