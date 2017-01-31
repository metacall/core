/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
%typemap(in) (const char * name, ...)(void * vargs[16])
{
	/*void ** vargs;*/
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
	/*vargs = (void **) malloc(args_size * sizeof(void *));

	if (vargs == NULL)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Invalid argument allocation",
			NewStringType::kNormal).ToLocalChecked());

		SWIG_fail;

		return;
	}*/

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		Local<Value> js_arg = args[args_count + 1];

		if (js_arg->IsBoolean())
		{
			boolean b = (js_arg->BooleanValue() == true) ? 1L : 0L;

 			vargs[args_count] = metacall_value_create_bool(b);
		}
		else if (js_arg->IsInt32())
		{
			/* Assume int is at least 32-bit width */
			int i = (int)js_arg->Int32Value();

			vargs[args_count] = metacall_value_create_int(i);
		}
		/*else if (js_arg->IsInteger())
		{
			*//* Assume long is at least 64-bit width *//*
			long l = (long)js_arg->IntegerValue();

			vargs[args_count] = metacall_value_create_long(l);
		}*/
		else if (js_arg->IsNumber())
		{
			double d = js_arg->NumberValue();

			vargs[args_count] = metacall_value_create_double(d);
		}
		else if (js_arg->IsString())
		{
			String::Utf8Value str(js_arg->ToString(args.GetIsolate()));

			vargs[args_count] = metacall_value_create_string(*str, str.length());
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
	void ** vargs;
	void * ret;

	args_size = args.Length() - 1;
	vargs = (void **) arg2;

	/* Execute call */
	ret = metacallv(arg1, vargs);

	/* Clear args */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		metacall_value_destroy(vargs[args_count]);
	}

	/* TODO: Remove this by a local array? */
	/*free(vargs);*/

	/* Return value */
	if (ret != NULL)
	{
		switch (metacall_value_id(ret))
		{

			case METACALL_BOOL :
			{
				bool b = ((long)metacall_value_to_bool(ret) == 1L ? true : false);

				$result = Boolean::New(args.GetIsolate(), b);

				break;
			}

			case METACALL_CHAR :
			{
				char c = metacall_value_to_char(ret);

				$result = String::NewFromUtf8(args.GetIsolate(), &c, String::kNormalString, 1);

				break;
			}

			case METACALL_SHORT :
			{
				short s = metacall_value_to_short(ret);

				$result = Integer::New(args.GetIsolate(), (int32_t)s);

				break;
			}

			case METACALL_INT :
			{
				int i = metacall_value_to_int(ret);

				$result = Integer::New(args.GetIsolate(), (int32_t)i);

				break;
			}

			case METACALL_LONG :
			{
				long l = metacall_value_to_long(ret);

				/* TODO: Check cast... */
				$result = Integer::New(args.GetIsolate(), (int32_t)l);

				break;
			}

			case METACALL_FLOAT :
			{
				float f = metacall_value_to_float(ret);

				$result = Number::New(args.GetIsolate(), (double)f);

				break;
			}

			case METACALL_DOUBLE :
			{
				double d = metacall_value_to_double(ret);

				$result = Number::New(args.GetIsolate(), d);

				break;
			}

			case METACALL_STRING :
			{
				const char * str = metacall_value_to_string(ret);

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

		metacall_value_destroy(ret);
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
