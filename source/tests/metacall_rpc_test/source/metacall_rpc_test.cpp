/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_rpc_test : public testing::Test
{
public:
};

TEST_F(metacall_rpc_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* RPC */
#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = {
			"remote.url"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, sizeof(rpc_scripts) / sizeof(rpc_scripts[0]), &handle));

		/* Print inspect information */
		{
			size_t size = 0;

			struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

			void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

			char *inspect_str = metacall_inspect(&size, allocator);

			EXPECT_NE((char *)NULL, (char *)inspect_str);

			EXPECT_GT((size_t)size, (size_t)0);

			static const char inspect_data[] = "{\"__metacall_host__\":[],\"rpc\":[{\"name\":\"remote.url\",\"scope\":{\"name\":\"global_namespace\",\"funcs\":[{\"name\":\"say_null\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[]},\"async\":false},{\"name\":\"get_second_untyped\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"first\",\"type\":{\"name\":\"\",\"id\":18}},{\"name\":\"second\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"hello\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[]},\"async\":false},{\"name\":\"two_doubles\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Double\",\"id\":6}},\"args\":[{\"name\":\"first_parameter\",\"type\":{\"name\":\"Double\",\"id\":6}},{\"name\":\"second_parameter\",\"type\":{\"name\":\"Double\",\"id\":6}}]},\"async\":false},{\"name\":\"say_sum_ducktyped\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"\",\"id\":18}},{\"name\":\"right\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"my_empty_func_str\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[]},\"async\":false},{\"name\":\"get_second\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"first\",\"type\":{\"name\":\"Integer\",\"id\":3}},{\"name\":\"second\",\"type\":{\"name\":\"Integer\",\"id\":3}}]},\"async\":false},{\"name\":\"dont_load_this_function\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"\",\"id\":18}},{\"name\":\"right\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"divide\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Double\",\"id\":6}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"Double\",\"id\":6}},{\"name\":\"right\",\"type\":{\"name\":\"Double\",\"id\":6}}]},\"async\":false},{\"name\":\"bytebuff\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Buffer\",\"id\":8}},\"args\":[{\"name\":\"input\",\"type\":{\"name\":\"Buffer\",\"id\":8}}]},\"async\":false},{\"name\":\"return_array\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[]},\"async\":false},{\"name\":\"Concat\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[{\"name\":\"a\",\"type\":{\"name\":\"String\",\"id\":7}},{\"name\":\"b\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"say_hello\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"value\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"say_multiply_ducktyped\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"\",\"id\":18}},{\"name\":\"right\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"mixed_args\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Char\",\"id\":1}},\"args\":[{\"name\":\"a_char\",\"type\":{\"name\":\"Char\",\"id\":1}},{\"name\":\"b_int\",\"type\":{\"name\":\"Integer\",\"id\":3}},{\"name\":\"c_long\",\"type\":{\"name\":\"Long\",\"id\":4}},{\"name\":\"d_double\",\"type\":{\"name\":\"Double\",\"id\":6}},{\"name\":\"e_ptr\",\"type\":{\"name\":\"Ptr\",\"id\":11}}]},\"async\":false},{\"name\":\"sum\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Long\",\"id\":4}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"Long\",\"id\":4}},{\"name\":\"right\",\"type\":{\"name\":\"Long\",\"id\":4}}]},\"async\":false},{\"name\":\"Sum\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Integer\",\"id\":3}},\"args\":[{\"name\":\"a\",\"type\":{\"name\":\"Integer\",\"id\":3}},{\"name\":\"b\",\"type\":{\"name\":\"Integer\",\"id\":3}}]},\"async\":false},{\"name\":\"my_empty_func_int\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Integer\",\"id\":3}},\"args\":[]},\"async\":false},{\"name\":\"strcat\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"String\",\"id\":7}},{\"name\":\"right\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"Say\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"text\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"backwardsPrime\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"start\",\"type\":{\"name\":\"\",\"id\":18}},{\"name\":\"stop\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"return_same_array\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"arr\",\"type\":{\"name\":\"\",\"id\":18}}]},\"async\":false},{\"name\":\"two_str\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[{\"name\":\"a_str\",\"type\":{\"name\":\"String\",\"id\":7}},{\"name\":\"b_str\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"SayHello\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[]},\"async\":false},{\"name\":\"say_string_without_spaces\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"value\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"say_multiply\",\"signature\":{\"ret\":{\"type\":{\"name\":\"\",\"id\":18}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"Integer\",\"id\":3}},{\"name\":\"right\",\"type\":{\"name\":\"Integer\",\"id\":3}}]},\"async\":false},{\"name\":\"multiply\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Long\",\"id\":4}},\"args\":[{\"name\":\"left\",\"type\":{\"name\":\"Long\",\"id\":4}},{\"name\":\"right\",\"type\":{\"name\":\"Long\",\"id\":4}}]},\"async\":false},{\"name\":\"new_args\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[{\"name\":\"a_str\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false},{\"name\":\"my_empty_func\",\"signature\":{\"ret\":{\"type\":{\"name\":\"Integer\",\"id\":3}},\"args\":[]},\"async\":false},{\"name\":\"three_str\",\"signature\":{\"ret\":{\"type\":{\"name\":\"String\",\"id\":7}},\"args\":[{\"name\":\"a_str\",\"type\":{\"name\":\"String\",\"id\":7}},{\"name\":\"b_str\",\"type\":{\"name\":\"String\",\"id\":7}},{\"name\":\"c_str\",\"type\":{\"name\":\"String\",\"id\":7}}]},\"async\":false}],\"classes\":[],\"objects\":[]}}]}";

			EXPECT_EQ((int)0, (int)strcmp(inspect_str, inspect_data));

			std::cout << inspect_str << std::endl;

			metacall_allocator_free(allocator, inspect_str);

			metacall_allocator_destroy(allocator);
		}

		const enum metacall_value_id divide_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT
		};

		void *ret = metacallt("divide", divide_ids, 50.0f, 10.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_to_float(ret), (float)5.0f);

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle));

		static const char buffer[] = "http://localhost:6094/viferga/example/v1";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rpc", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_RPC */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
