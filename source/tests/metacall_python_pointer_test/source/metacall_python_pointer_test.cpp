/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_python_pointer_test : public testing::Test
{
public:
};

struct test_type
{
	long value;
};

void * native_set_value(void * args[])
{
	struct test_type * t  = (struct test_type *)metacall_value_to_ptr(args[0]);
	long value = metacall_value_to_long(args[1]);

	printf("Pointer: %p\n", (void *)t);
	printf("Value: %ld\n", value);

	t->value = value;

	return metacall_value_create_ptr((void *)t);
}

TEST_F(metacall_python_pointer_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Native register */
	{
		metacall_register("native_set_value", native_set_value, METACALL_PTR, 2, METACALL_PTR, METACALL_LONG);

		EXPECT_NE((void *) NULL, (void *) metacall_function("native_set_value"));
	}

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"pointer.py"
		};

		void * ret = NULL;

		struct test_type t = { 0 };

		void * t_ptr = (void *)&t;

		long value = 3000;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		const enum metacall_value_id ids[] =
		{
			METACALL_PTR, METACALL_LONG
		};

		ret = metacallt("python_set_value", ids, t_ptr, value);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((void *) t_ptr, (void *) metacall_value_cast_ptr(&ret));

		EXPECT_EQ((long) value, (long) t.value);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
