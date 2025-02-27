/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_python_exception_test : public testing::Test
{
public:
};

TEST_F(metacall_python_exception_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"def py_throw_error():\n"
			"	raise TypeError('yeet')\n"
			"\n"
			"def py_return_error():\n"
			"	return BaseException('asdf')\n"
			"\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		void *ret = metacall("py_throw_error");

		struct metacall_exception_type ex;

		EXPECT_EQ((int)0, (int)metacall_error_from_value(ret, &ex));

		EXPECT_EQ((int)0, (int)strcmp("yeet", ex.message));

		EXPECT_EQ((int)0, (int)strcmp("TypeError", ex.label));

		metacall_value_destroy(ret);

		ret = metacall("py_return_error");

		EXPECT_EQ((int)0, (int)metacall_error_from_value(ret, &ex));

		EXPECT_EQ((int)0, (int)strcmp("asdf", ex.message));

		EXPECT_EQ((int)0, (int)strcmp("BaseException", ex.label));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
