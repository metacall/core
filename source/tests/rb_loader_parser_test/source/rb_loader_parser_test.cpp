/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <gtest/gtest.h>

#include <rb_loader/rb_loader_impl_parser.h>

#include <log/log.h>

class rb_loader_parser_test : public testing::Test
{
protected:
};

TEST_F(rb_loader_parser_test, DefaultConstructor)
{
	const char script[] =
		"#!/usr/bin/ruby\n"

		"@@dic={}\n"

		"def asd(a)\n"
		"	@@dic={}\n"
		"	puts 'ruby: initialize'\n"
		"end\n"

		"def cache_initialize()\n"
		"	@@dic={}\n"
		"	puts 'ruby: initialize'\n"
		"end\n"

		"def cache_has_key(key: String)\n"
		"	puts 'ruby: has key'\n"
		"	return @@dic.has_key? key\n"
		"end\n"

		"def cache_set(key: String, value: String)\n"
		"	puts 'ruby: set value'\n"
		"	@@dic[key] = value\n"
		"end\n"

		"def cache_get(key: String)\n"
		"	puts 'ruby: get value'\n"
		"	return @@dic[key]\n"
		"end\n"

		"def ducktype(key)\n"
		"	puts 'ruby: get value'\n"
		"	return @@dic[key]\n"
		"end\n"

		"def ducktype_multi(key, a, b, c, d)\n"
		"	puts 'ruby: get value'\n"
		"	return @@dic[key]\n"
		"end\n"

		"def cache_nothing(key: String)\n"
		"end\n";

	set function_map;

	rb_function_parser function_parser;

	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	function_map = set_create(&hash_callback_str, &comparable_callback_str);

	EXPECT_NE((set)NULL, (set)function_map);

	EXPECT_EQ((int)0, (int)rb_loader_impl_key_parse(script, function_map));

	rb_loader_impl_key_print(function_map);

	/* cache_nothing */
	{
		const char cache_nothing[] = "cache_nothing";

		function_parser = (rb_function_parser)set_get(function_map, (set_key)cache_nothing);

		EXPECT_NE((rb_function_parser)NULL, (rb_function_parser)function_parser);

		EXPECT_EQ((int)0, (int)strncmp(function_parser->name, cache_nothing, RB_LOADER_IMPL_PARSER_FUNC));

		EXPECT_EQ((size_t)1, (size_t)function_parser->params_size);

		EXPECT_EQ((int)0, (int)function_parser->params[0].index);
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].name, "key", RB_LOADER_IMPL_PARSER_KEY));
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].type, "String", RB_LOADER_IMPL_PARSER_TYPE));
	}

	/* cache_get */
	{
		const char cache_get[] = "cache_get";

		function_parser = (rb_function_parser)set_get(function_map, (set_key)cache_get);

		EXPECT_NE((rb_function_parser)NULL, (rb_function_parser)function_parser);

		EXPECT_EQ((int)0, (int)strncmp(function_parser->name, cache_get, RB_LOADER_IMPL_PARSER_FUNC));

		EXPECT_EQ((size_t)1, (size_t)function_parser->params_size);

		EXPECT_EQ((int)0, (int)function_parser->params[0].index);
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].name, "key", RB_LOADER_IMPL_PARSER_KEY));
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].type, "String", RB_LOADER_IMPL_PARSER_TYPE));
	}

	/* cache_set */
	{
		const char cache_set[] = "cache_set";

		function_parser = (rb_function_parser)set_get(function_map, (set_key)cache_set);

		EXPECT_NE((rb_function_parser)NULL, (rb_function_parser)function_parser);

		EXPECT_EQ((int)0, (int)strncmp(function_parser->name, cache_set, RB_LOADER_IMPL_PARSER_FUNC));

		EXPECT_EQ((size_t)2, (size_t)function_parser->params_size);

		EXPECT_EQ((int)0, (int)function_parser->params[0].index);
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].name, "key", RB_LOADER_IMPL_PARSER_KEY));
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].type, "String", RB_LOADER_IMPL_PARSER_TYPE));

		EXPECT_EQ((int)1, (int)function_parser->params[1].index);
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[1].name, "value", RB_LOADER_IMPL_PARSER_KEY));
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[1].type, "String", RB_LOADER_IMPL_PARSER_TYPE));
	}

	/* cache_has_key */
	{
		const char cache_has_key[] = "cache_has_key";

		function_parser = (rb_function_parser)set_get(function_map, (set_key)cache_has_key);

		EXPECT_NE((rb_function_parser)NULL, (rb_function_parser)function_parser);

		EXPECT_EQ((int)0, (int)strncmp(function_parser->name, cache_has_key, RB_LOADER_IMPL_PARSER_FUNC));

		EXPECT_EQ((size_t)1, (size_t)function_parser->params_size);

		EXPECT_EQ((int)0, (int)function_parser->params[0].index);
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].name, "key", RB_LOADER_IMPL_PARSER_KEY));
		EXPECT_EQ((int)0, (int)strncmp(function_parser->params[0].type, "String", RB_LOADER_IMPL_PARSER_TYPE));
	}

	/* cache_initialize */
	{
		const char cache_initialize[] = "cache_initialize";

		function_parser = (rb_function_parser)set_get(function_map, (set_key)cache_initialize);

		EXPECT_NE((rb_function_parser)NULL, (rb_function_parser)function_parser);

		EXPECT_EQ((int)0, (int)strncmp(function_parser->name, cache_initialize, RB_LOADER_IMPL_PARSER_FUNC));

		EXPECT_EQ((size_t)0, (size_t)function_parser->params_size);
	}

	EXPECT_EQ((int)0, (int)rb_loader_impl_key_clear(function_map));

	set_destroy(function_map);
}
