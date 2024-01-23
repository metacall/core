/*
 *	Format Library by Parra Studios
 *	A cross-platform library for supporting platform specific environment features.
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

#include <environment/environment.h>

class environment_test : public testing::Test
{
public:
};

#define ENVIRONMENT_TEST_VARIABLE_TEXT			 "ENV_TEXT"
#define ENVIRONMENT_TEST_VARIABLE_PATH			 "ENV_PATH"
#define ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED "ENV_SAN"

TEST_F(environment_test, variable_text)
{
	static const char variable_text_name[] = ENVIRONMENT_TEST_VARIABLE_TEXT;

	char *variable_text = environment_variable_create(variable_text_name, "default");

	ASSERT_NE((const char *)NULL, (const char *)variable_text);

	EXPECT_EQ((int)0, (int)strcmp(variable_text, "abcd"));

	environment_variable_destroy(variable_text);
}

TEST_F(environment_test, variable_text_default)
{
	static const char variable_text_name[] = "UNKNOWN";

	char *variable_text = environment_variable_create(variable_text_name, "default");

	ASSERT_NE((const char *)NULL, (const char *)variable_text);

	EXPECT_EQ((int)0, (int)strcmp(variable_text, "default"));

	environment_variable_destroy(variable_text);
}

TEST_F(environment_test, variable_static)
{
	static const char variable_text_name[] = ENVIRONMENT_TEST_VARIABLE_TEXT;

	const char *variable_text_static = environment_variable_get(variable_text_name, "default");

	EXPECT_EQ((int)0, (int)strcmp(variable_text_static, "abcd"));
}

TEST_F(environment_test, variable_path)
{
	static const char variable_path_name[] = ENVIRONMENT_TEST_VARIABLE_PATH;

	char *variable_path = environment_variable_path_create(variable_path_name, "default_path", sizeof("default_path"), NULL);

	ASSERT_NE((const char *)NULL, (const char *)variable_path);

	EXPECT_EQ((int)0, (int)strcmp(variable_path, "abcd" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}

TEST_F(environment_test, variable_path_default)
{
	static const char variable_path_name[] = "UNKNOWN";

	char *variable_path = environment_variable_path_create(variable_path_name, "default_path", sizeof("default_path"), NULL);

	ASSERT_NE((const char *)NULL, (const char *)variable_path);

	EXPECT_EQ((int)0, (int)strcmp(variable_path, "default_path" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}

TEST_F(environment_test, variable_path_sanitized)
{
	static const char variable_path_name[] = ENVIRONMENT_TEST_VARIABLE_PATH_SANITIZED;

	char *variable_path = environment_variable_path_create(variable_path_name, "default_path", sizeof("default_path"), NULL);

	ASSERT_NE((const char *)NULL, (const char *)variable_path);

	EXPECT_EQ((int)0, (int)strcmp(variable_path, "abcd" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR));

	environment_variable_path_destroy(variable_path);
}
