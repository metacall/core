/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_python_open_test : public testing::Test
{
public:
};

TEST_F(metacall_python_open_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"landing.py"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		void * ret = metacall("index");

		EXPECT_NE((void *) NULL, (void *) ret);

		const char * result = metacall_value_to_string(ret);

		EXPECT_NE((int) 0, (int) strcmp(result, "<html><head></head><body>Error</body></html>"));

		metacall_value_destroy(ret);

		static const char str[] = "Hello World";

		void * args[] =
		{
			metacall_value_create_string(str, sizeof(str) - 1)
		};

		ret = metacallv("login", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		const char * token = metacall_value_to_string(ret);

		EXPECT_EQ((int) 0, (int) strcmp(token, "eyJhbGciOiJIUzI1NiJ9.SGVsbG8gV29ybGQ.Iyc6PWVbK538giVdaInTeIO3jvvC1Vuy_czZUzoRRec"));

		metacall_value_destroy(args[0]);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char * inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *) NULL, (char *) inspect_str);

		EXPECT_GT((size_t) size, (size_t) 0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
