/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.hpp>

#include <stdlib.h>
#include <string.h>
#include <map>
#include <set>
#include <string>
#include <utility>

using namespace metacall;
using namespace metacall::detail;

class metacall_loader_types_test : public testing::Test
{
public:
};

TEST_F(metacall_loader_types_test, TypeRegistryUniqueEnforcement)
{
	loader_impl impl = loader_impl_create_host("test");

	ASSERT_NE(impl, (loader_impl)NULL);

	/* 1. Registering a native name once succeeds */
	type t1 = type_create(TYPE_INT, "number", NULL, NULL);
	ASSERT_NE(t1, (type)NULL);
	ASSERT_EQ(0, loader_impl_type_define(impl, "number", t1));
	EXPECT_EQ(t1, loader_impl_type(impl, "number"));
	EXPECT_EQ(TYPE_INT, type_index(loader_impl_type(impl, "number")));

	/* 2. Registering the same native name again fails */
	type t2 = type_create(TYPE_DOUBLE, "number", NULL, NULL);
	ASSERT_NE(t2, (type)NULL);
	EXPECT_NE(0, loader_impl_type_define(impl, "number", t2));

	/* 3. The original registration remains intact after a duplicate attempt */
	EXPECT_EQ(t1, loader_impl_type(impl, "number"));
	EXPECT_EQ(TYPE_INT, type_index(loader_impl_type(impl, "number")));

	/* 4. Failed registration does not leak the incoming type object:
	 * The caller owns and destroys the rejected type object */
	type_destroy(t2);

	/* Exact duplicate registration (same name + same ID) must also be rejected */
	type t_dup = type_create(TYPE_INT, "number", NULL, NULL);
	ASSERT_NE(t_dup, (type)NULL);
	EXPECT_NE(0, loader_impl_type_define(impl, "number", t_dup));
	type_destroy(t_dup);

	/* Original registration remains intact */
	EXPECT_EQ(t1, loader_impl_type(impl, "number"));

	/* 5. Registering a second, distinct native name succeeds */
	type t3 = type_create(TYPE_STRING, "string", NULL, NULL);
	ASSERT_NE(t3, (type)NULL);
	ASSERT_EQ(0, loader_impl_type_define(impl, "string", t3));
	EXPECT_EQ(t3, loader_impl_type(impl, "string"));
	EXPECT_EQ(TYPE_STRING, type_index(loader_impl_type(impl, "string")));

	/* 6. Verify types set size and contents */
	detail::set types = loader_impl_types(impl);
	ASSERT_NE(types, (detail::set)NULL);
	EXPECT_EQ((size_t)2, set_size(types));

	/* Clean destruction of loader (destroys registered t1 and t3 without leak or double free) */
	loader_impl_destroy(NULL, impl);
}

#if defined(OPTION_BUILD_LOADERS_PY)
TEST_F(metacall_loader_types_test, PythonLoaderTypes)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_loader_type *types = NULL;
	size_t size = 0;
	bool check_bool_type_exists = false;

	EXPECT_EQ((int)0, (int)metacall_loader_types("py", &types, &size));

	ASSERT_GT(size, (size_t)0);
	ASSERT_NE(types, (struct metacall_loader_type *)NULL);

	for (size_t i = 0; i < size; ++i)
	{
		EXPECT_NE(types[i].name, (const char *)NULL);
		EXPECT_GE((int)types[i].id, 0);

		std::string name_str(types[i].name);

		if (name_str == "bool")
		{
			check_bool_type_exists = true;
		}
	}

	ASSERT_EQ(check_bool_type_exists, true);

	free(types);

	metacall_destroy();
}
#endif

#if defined(OPTION_BUILD_LOADERS_LUA)
TEST_F(metacall_loader_types_test, LuaLoaderMetadata)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_loader_type *types = NULL;
	size_t size = 0;

	EXPECT_EQ((int)0, (int)metacall_loader_types("lua", &types, &size));

	ASSERT_NE(types, (struct metacall_loader_type *)NULL);

	std::set<std::pair<std::string, enum metacall_value_id>> registrations;

	for (size_t i = 0; i < size; ++i)
	{
		EXPECT_NE(types[i].name, (const char *)NULL);
		EXPECT_GE((int)types[i].id, 0);

		registrations.insert(std::make_pair(std::string(types[i].name), types[i].id));
	}

	/* Verify common canonical registrations */
	EXPECT_NE(registrations.find(std::make_pair("boolean", METACALL_BOOL)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("number", METACALL_DOUBLE)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("string", METACALL_STRING)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("table", METACALL_MAP)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("nil", METACALL_NULL)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("function", METACALL_FUNCTION)), registrations.end());
	EXPECT_NE(registrations.find(std::make_pair("userdata", METACALL_PTR)), registrations.end());

	/* Verify C++ wrapper metacall::loader_types("lua") */
	std::map<std::string, enum metacall_value_id> lua_map = metacall::loader_types("lua");
	EXPECT_EQ(METACALL_BOOL, lua_map["boolean"]);
	EXPECT_EQ(METACALL_DOUBLE, lua_map["number"]);
	EXPECT_EQ(METACALL_STRING, lua_map["string"]);
	EXPECT_EQ(METACALL_MAP, lua_map["table"]);
	EXPECT_EQ(METACALL_NULL, lua_map["nil"]);
	EXPECT_EQ(METACALL_FUNCTION, lua_map["function"]);
	EXPECT_EQ(METACALL_PTR, lua_map["userdata"]);

	/* Lua 5.3+ distinguishes integer from number:
	 * For Lua 5.3+, "integer" -> METACALL_LONG is registered (total 8 registrations).
	 * For Lua 5.1/5.2, "integer" is absent and "number" is DOUBLE (total 7 registrations).
	 */
	if (registrations.find(std::make_pair("integer", METACALL_LONG)) != registrations.end())
	{
		EXPECT_EQ((size_t)8, size);
		EXPECT_EQ(METACALL_LONG, lua_map["integer"]);
	}
	else
	{
		EXPECT_EQ((size_t)7, size);
		EXPECT_EQ(lua_map.find("integer"), lua_map.end());
	}

	free(types);

	metacall_destroy();
}

TEST_F(metacall_loader_types_test, LuaConversions)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char script[] =
		"function lua_int_val()\n"
		"    return 42\n"
		"end\n"
		"function lua_float_val()\n"
		"    return 3.14\n"
		"end\n"
		"function lua_add(a, b)\n"
		"    return a + b\n"
		"end\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("lua", script, sizeof(script), NULL));

	/* Verify integer conversion */
	auto ret_int = metacall::metacall<metacall::value>("lua_int_val");
	EXPECT_EQ(METACALL_LONG, ret_int.id());
	EXPECT_EQ(42L, metacall_value_to_long(ret_int.to_raw()));

	/* Verify floating-point conversion */
	auto ret_float = metacall::metacall<metacall::value>("lua_float_val");
	EXPECT_EQ(METACALL_DOUBLE, ret_float.id());
	EXPECT_DOUBLE_EQ(3.14, metacall_value_to_double(ret_float.to_raw()));

	metacall_destroy();
}
#endif
