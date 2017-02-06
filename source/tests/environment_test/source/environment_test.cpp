/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting formatted input / output.
 *
 */

#include <gmock/gmock.h>

#include <environment/environment.h>

class environment_test : public testing::Test
{
public:
};

#define ENVIRONMENT_TEST_VARIABLE_TEXT "ENVIRONMENT_TEST_VARIABLE_TEXT"
#define ENVIRONMENT_TEST_VARIABLE_PATH "ENVIRONMENT_TEST_VARIABLE_PATH"
#define ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED "ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED"

TEST_F(environment_test, variable_text)
{
	static const char variable_text_name[] = ENVIRONMENT_TEST_VARIABLE_TEXT;

	char * variable_text = environment_variable_create(variable_text_name, "default");

	ASSERT_NE((const char *) NULL, (const char *) variable_text);

	EXPECT_EQ((int) 0, (int) strcmp(variable_text, "abcd"));

	environment_variable_destroy(variable_text);
}

TEST_F(environment_test, variable_text_default)
{
	static const char variable_text_name[] = "UNKNOWN";

	char * variable_text = environment_variable_create(variable_text_name, "default");

	ASSERT_NE((const char *) NULL, (const char *) variable_text);

	EXPECT_EQ((int) 0, (int) strcmp(variable_text, "default"));

	environment_variable_destroy(variable_text);
}

TEST_F(environment_test, variable_static)
{
	static const char variable_text_name[] = ENVIRONMENT_TEST_VARIABLE_TEXT;

	const char * variable_text_static = environment_variable_get(variable_text_name, "default");

	EXPECT_EQ((int) 0, (int) strcmp(variable_text_static, "abcd"));
}

TEST_F(environment_test, variable_path)
{
	static const char variable_path_name[] = ENVIRONMENT_TEST_VARIABLE_PATH;

	char * variable_path = environment_variable_path_create(variable_path_name, "default_path");

	ASSERT_NE((const char *) NULL, (const char *) variable_path);
	
	#if defined(_WIN32)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd\\"));
	#elif defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd/"));
	#endif

	environment_variable_path_destroy(variable_path);
}

TEST_F(environment_test, variable_path_default)
{
	static const char variable_path_name[] = "UNKNOWN";

	char * variable_path = environment_variable_path_create(variable_path_name, "default_path");

	ASSERT_NE((const char *) NULL, (const char *) variable_path);
	
	#if defined(_WIN32)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "default_path\\"));
	#elif defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "default_path/"));
	#endif

	environment_variable_path_destroy(variable_path);
}


TEST_F(environment_test, variable_path_sanitized)
{
	static const char variable_path_name[] = ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED;

	char * variable_path = environment_variable_path_create(variable_path_name, "default_path");

	ASSERT_NE((const char *) NULL, (const char *) variable_path);
	
	#if defined(_WIN32)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd\\"));
	#elif defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd/"));
	#endif

	environment_variable_path_destroy(variable_path);
}
