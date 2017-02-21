/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <gmock/gmock.h>

#include <rb_loader/rb_loader_impl_parser.h>

#include <log/log.h>

class rb_loader_parser_test : public testing::Test
{
  protected:
};

TEST_F(rb_loader_parser_test, DefaultConstructor)
{
	/*
	const char script[] =
		"#!/usr/bin/ruby\n"

		"def say_hello_inline(value: String)\n"
		"	result = 'Hello ' + value + '!'\n"
		"	puts(result)\n"
		"	return result\n"
		"end\n"

		"def say_multiply_inline(left: Fixnum, right: Fixnum)\n"
		"	result = left * right\n"
		"	puts('Multiply', result, '!')\n"
		"	return result\n"
		"end\n"

		"def say_null_inline()\n"
		"	puts('Helloooo from null method!')\n"
		"end\n";
	*/

	const char script_crash[] =
		"#!/usr/bin/ruby\n"

		"@@dic={}\n"

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

		"def cache_nothing(key: String)\n"
		"end\n";

	set function_map;

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	function_map = set_create(&hash_callback_str, &comparable_callback_str);

	EXPECT_NE((set) NULL, (set) function_map);

	EXPECT_EQ((int) 0, (int) rb_loader_impl_key_parse(script_crash, function_map));

	rb_loader_impl_key_print(function_map);

	EXPECT_EQ((int) 0, (int) rb_loader_impl_key_clear(function_map));

	set_destroy(function_map);
}
