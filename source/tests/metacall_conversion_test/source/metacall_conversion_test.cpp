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

#include <metacall/metacall_loaders.h>
#include <metacall/metacall.hpp>

#include <cstdlib>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * Generates the type table of each loader, with the following format:
 * {
 *   "<tag>": {
 *     "types": { "<native type name>": "<metacall type name>", ... },
 *     "mapping": { "<metacall type name>": "<metacall type name returned by identity>", ... }
 *   }, ...
 * }
 */

class metacall_conversion_test : public testing::Test
{
public:
};

/* Type names indexed by type name */
using type_map = metacall::map_typed<std::string, std::string>;

/* Types and mapping of a loader */
using loader_table = metacall::map_typed<std::string, type_map>;

/* Loader tables indexed by loader tag */
using type_table = metacall::map_typed<std::string, loader_table>;

TEST_F(metacall_conversion_test, DefaultConstructor)
{
	metacall::metacall_print_info();

	metacall::metacall_log_null();

	ASSERT_EQ((int)0, (int)metacall::metacall_initialize());

	/* Each script defines identity_<tag>, so all of them can be loaded into the global scope */
	const std::map<std::string, std::string> scripts = {
#if defined(OPTION_BUILD_LOADERS_NODE)
		{ "node",
			"module.exports = { identity_node: x => x };\n" },
#endif

#if defined(OPTION_BUILD_LOADERS_PY)
		{ "py",
			"def identity_py(x):\n"
			"	return x\n" },
#endif

#if defined(OPTION_BUILD_LOADERS_RB)
		{ "rb",
			"def identity_rb(x)\n"
			"	x\n"
			"end\n" },
#endif

#if defined(OPTION_BUILD_LOADERS_LUA)
		{ "lua",
			"function identity_lua(x)\n"
			"	return x\n"
			"end\n" }
#endif
	};

	std::vector<type_table::entry> loaders;

	for (const auto &[tag, script] : scripts)
	{
		const std::string identity = "identity_" + tag;
		std::vector<type_map::entry> types, mapping;

		ASSERT_EQ((int)0, (int)metacall::metacall_load_from_memory(tag.c_str(), script.c_str(), script.size() + 1, NULL));

		/* Native type names mapped to their type id */
		for (const auto &[native, id] : metacall::loader_types(tag))
		{
			types.emplace_back(native, metacall::metacall_value_id_name(id));
		}

		/* Round trip of each type id through the loader */
		for (size_t id = 0; id < metacall::METACALL_SIZE; ++id)
		{
			const auto value_id = static_cast<enum metacall::metacall_value_id>(id);
			const char *name = metacall::metacall_value_id_name(value_id);

			metacall::value_typed<metacall::value> arg(metacall::metacall_value_create(value_id), &metacall::metacall_value_destroy);

			try
			{
				auto ret = metacall::metacall<metacall::value>(identity, std::move(arg));

				mapping.emplace_back(name, metacall::metacall_value_id_name(ret.id()));
			}
			catch (const std::runtime_error &)
			{
				mapping.emplace_back(name, metacall::metacall_value_id_name(metacall::METACALL_INVALID));
			}
		}

		loaders.emplace_back(tag, loader_table{ { "types", type_map(std::move(types)) }, { "mapping", type_map(std::move(mapping)) } });
	}

	type_table table(std::move(loaders));

	metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall::metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	size_t json_size = 0;

	char *json = metacall::metacall_serialize("rapid_json", table.to_raw(), &json_size, allocator);

	ASSERT_NE((char *)NULL, (char *)json);

	std::cout << json << std::endl;

	std::ofstream file(METACALL_TYPE_TABLE_PATH);

	ASSERT_TRUE(file.is_open());

	file << json;

	file.close();

	metacall::metacall_allocator_free(allocator, json);

	metacall::metacall_allocator_destroy(allocator);
}
