/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#define ENVIRONMENT_TEST_VARIABLE_TEXT "ENV_TEXT"
#define ENVIRONMENT_TEST_VARIABLE_PATH "ENV_PATH"
#define ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED "ENV_SAN"

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
	
	EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}

TEST_F(environment_test, variable_path_default)
{
	static const char variable_path_name[] = "UNKNOWN";

	char * variable_path = environment_variable_path_create(variable_path_name, "default_path");

	ASSERT_NE((const char *) NULL, (const char *) variable_path);

	EXPECT_EQ((int) 0, (int) strcmp(variable_path, "default_path" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}


TEST_F(environment_test, variable_path_sanitized)
{
	static const char variable_path_name[] = ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED;

	char * variable_path = environment_variable_path_create(variable_path_name, "default_path");

	ASSERT_NE((const char *) NULL, (const char *) variable_path);

	EXPECT_EQ((int) 0, (int) strcmp(variable_path, "abcd" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}
