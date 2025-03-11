/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_python_loader_port_test : public testing::Test
{
public:
};

void *callback_host(size_t argc, void *args[], void *data)
{
	const char *str = metacall_value_cast_string(&args[0]);

	(void)argc;
	(void)data;

	printf("Host callback: %s\n", str);

	EXPECT_EQ((int)0, (int)strcmp(str, "some text"));

	return metacall_value_create_int(25);
}

TEST_F(metacall_python_loader_port_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Native register */
	{
		metacall_register("callback_host_impl", callback_host, NULL, METACALL_INT, 1, METACALL_STRING);

		EXPECT_NE((void *)NULL, (void *)metacall_function("callback_host_impl"));
	}

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char *rb_scripts[] = {
			"hello.rb", "second.rb"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0]), NULL));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)35, (int)metacall_value_cast_int(&ret));

		metacall_value_destroy(ret);

		ret = metacall("say_null");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_cast_string(&ret), "Hello meta-programmer!"));

		metacall_value_destroy(ret);

		EXPECT_NE((void *)NULL, (void *)metacall_function("get_second"));

		ret = metacall("get_second", 5, 12);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)12, (int)metacall_value_cast_int(&ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"callback.py"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ret = metacall("hello_world", "some text");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)25, (int)metacall_value_cast_int(&ret));

		metacall_value_destroy(ret);

		ret = metacall("hello_ruby", 12, 4);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)48, (int)metacall_value_cast_int(&ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
