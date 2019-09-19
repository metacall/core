/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <cstdio>

class metacall_inspect_test : public testing::Test
{
public:
};

TEST_F(metacall_inspect_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* Ruby */
	#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char * rb_scripts[] =
		{
			"hello.rb", "second.rb"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_RB */

	/* JavaScript SpiderMonkey */
	#if defined(OPTION_BUILD_LOADERS_JSM)
	{
		const char * jsm_scripts[] =
		{
			"spider.jsm"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("jsm", jsm_scripts, sizeof(jsm_scripts) / sizeof(jsm_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_JSM */

	/* JavaScript V8 */
	#if defined(OPTION_BUILD_LOADERS_JS)
	{
		const char * js_scripts[] =
		{
			"divide.js", "third.js"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("js", js_scripts, sizeof(js_scripts) / sizeof(js_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_JS */

	/* Mock */
	#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		const char * mock_scripts[] =
		{
			"empty.mock"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("mock", mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_MOCK */

	/* C# Netcore */
	#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char * cs_scripts[] =
		{
			"hello.cs"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_CS */

	/* C */
	#if defined(OPTION_BUILD_LOADERS_C)
	{
		const char * c_scripts[] =
		{
			"compiled.c"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_C */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char * inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *) NULL, (char *) inspect_str);

		EXPECT_GT((size_t)size, (size_t) 0);

		printf("%s\n", inspect_str);

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
