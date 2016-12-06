/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_stringify.h>

#include <log/log.h>

class value_stringify_test : public testing::Test
{
  public:
};

TEST_F(value_stringify_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	static const char hello_world[] = "hello world";

	static const char * value_names[TYPE_SIZE] =
	{
		"true",
		"A",
		"123",
		"56464",
		"251251251",
		"545.345300",
		hello_world,
		#if defined(_WIN32) && defined(_MSC_VER)
			"0x000A7EF2"
		#elif defined(__linux) || defined(__linux__)
			"0xa7ef2"
		#else
			"<unknown>"
		#endif
	};

	value value_array[TYPE_SIZE] =
	{
		value_create_bool(1),
		value_create_char('A'),
		value_create_short(123),
		value_create_int(56464),
		value_create_long(251251251L),
		value_create_double(545.3453),
		value_create_string(hello_world, sizeof(hello_world)),
		value_create_ptr((void *)0x000A7EF2)
	};

	const size_t size = sizeof(value_names) / sizeof(value_names[0]);

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		#define BUFFER_SIZE 100

		char buffer[BUFFER_SIZE];

		const size_t size = BUFFER_SIZE;

		size_t length;

		value_stringify(value_array[iterator], buffer, size, &length);

		log_write("metacall", LOG_LEVEL_DEBUG, "(%s == %s)", buffer, value_names[iterator]);

		EXPECT_EQ((int) 0, (int) strcmp(buffer, value_names[iterator]));

		EXPECT_LT((size_t) length, (size_t) size);

		value_destroy(value_array[iterator]);

		#undef BUFFER_SIZE
	}
}
