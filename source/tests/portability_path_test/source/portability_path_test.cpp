/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <gtest/gtest.h>

#include <portability/portability_path.h>

#include <array>
#include <cstring>
#include <string>

#define NAME_SIZE ((size_t)PORTABILITY_PATH_SIZE / 2)
#define PATH_SIZE ((size_t)PORTABILITY_PATH_SIZE)

typedef char string_name[NAME_SIZE];
typedef char string_path[PATH_SIZE];

class portability_path_test : public testing::Test
{
public:
};

TEST_F(portability_path_test, portability_path_test_path_get_module_name)
{
	static const char base[] = "/a/b/c/asd.txt";
	static const char result[] = "asd";
	static const char extension[] = "txt";

	string_name name;

	size_t size = portability_path_get_module_name(base, sizeof(base), extension, sizeof(extension), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_module_name_without_extension)
{
	static const char base[] = "/a/b/c/asd";
	static const char result[] = "asd";
	static const char extension[] = "txt";

	string_name name;

	size_t size = portability_path_get_module_name(base, sizeof(base), extension, sizeof(extension), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_module_name_with_random_extension)
{
	static const char base[] = "/a/b/c/asd.etc.asd";
	static const char result[] = "asd.etc.asd";
	static const char extension[] = "txt";

	string_name name;

	size_t size = portability_path_get_module_name(base, sizeof(base), extension, sizeof(extension), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_null)
{
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(NULL, 0, name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_empty)
{
	static const char base[] = "";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name)
{
	static const char base[] = "/a/b/c/asd.txt";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_end_dot)
{
	static const char base[] = "/a/b/c/asd.";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_without_dot)
{
	static const char base[] = "/a/b/c/asd";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_dot_in_path)
{
	static const char base[] = "/a/b.c/d/asd";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_dot_in_path_and_name)
{
	static const char base[] = "/a/b.c/d/asd.txt";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_only_separator_dot)
{
	static const char base[] = "/.";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_only_dot)
{
	static const char base[] = "/a/b/c/.";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_two_dots)
{
	static const char base[] = "/a/b/c/..";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_three_dots)
{
	static const char base[] = "/a/b/c/...";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_only_extension)
{
	static const char base[] = "/a/b/c/.asd";
	static const char result[] = ".asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_double_extension)
{
	static const char base[] = "/a/b/c/.asd.yes";
	static const char result[] = ".asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_triple_extension)
{
	static const char base[] = "/a/b/c/.asd.yes.no";
	static const char result[] = ".asd.yes";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_nullchar)
{
	static const char base[] = "/home/yeet/.nvm/versions/node/v18.20.3/bin/node";
	static const char result[] = "node";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_path_of_path)
{
	static const char base[] = "/a/b/c/";
	static const char result[] = "/a/b/c/";

	string_path path;

	size_t size = portability_path_get_directory(base, sizeof(base), path, PATH_SIZE);

	EXPECT_STREQ(path, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_path_of_filepath)
{
	static const char base[] = "/a/b/c/asd";
	static const char result[] = "/a/b/c/";

	string_path path;

	size_t size = portability_path_get_directory(base, sizeof(base), path, PATH_SIZE);

	EXPECT_STREQ(path, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_relative)
{
	static const char base[] = "/a/b/c/";
	static const char path[] = "/a/b/c/abc";
	static const char result[] = "abc";

	string_path relative;

	size_t size = portability_path_get_relative(base, sizeof(base), path, sizeof(path), relative, PATH_SIZE);

	EXPECT_STREQ(relative, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_relative_fail)
{
	static const char base[] = "/this/is/not/shared/with/path";
	static const char path[] = "/a/b/c/abc";
	static const char result[] = "a/b/c/abc";

	string_path relative;

	size_t size = portability_path_get_relative(base, sizeof(base), path, sizeof(path), relative, PATH_SIZE);

	EXPECT_STREQ(relative, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_none_slash)
{
	static const char left[] = "/a/b/c";
	static const char right[] = "e/f/g/";
	static const char result[] = "/a/b/c/e/f/g/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_left_slash)
{
	static const char left[] = "/a/b/c/";
	static const char right[] = "e/f/g/";
	static const char result[] = "/a/b/c/e/f/g/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_right_slash)
{
	static const char left[] = "/a/b/c";
	static const char right[] = "/e/f/g/";
	static const char result[] = "/a/b/c/e/f/g/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_both_slash)
{
	static const char left[] = "/a/b/c/";
	static const char right[] = "/e/f/g/";
	static const char result[] = "/a/b/c/e/f/g/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_left_empty)
{
	static const char left[] = "";
	static const char right[] = "/e/f/g/";
	static const char result[] = "/e/f/g/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_right_empty)
{
	static const char left[] = "/a/b/c/";
	static const char right[] = "";
	static const char result[] = "/a/b/c/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_right_empty_non_slash)
{
	static const char left[] = "/a/b/c";
	static const char right[] = "";
	static const char result[] = "/a/b/c/";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_join_both_empty)
{
	static const char left[] = "";
	static const char right[] = "";
	static const char result[] = "";

	string_path join;

	size_t size = portability_path_join(left, sizeof(left), right, sizeof(right), join, PATH_SIZE);

	EXPECT_STREQ(join, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_dot)
{
	static const char path[] = "./a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_double_dot)
{
	static const char path[] = "../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_dot)
{
	static const char path[] = "./././././a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_double_dot)
{
	static const char path[] = "../../../../../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_dot_non_slash)
{
	static const char path[] = ".a/b/c";
	static const char result[] = ".a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_dot_non_slash)
{
	static const char path[] = "..a/b/c";
	static const char result[] = "..a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_invalid)
{
	static const char path[] = "..././.../...../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot)
{
	static const char path[] = "../a/b/../c";
	static const char result[] = "a/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot_all)
{
	static const char path[] = "../a/b/../../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot_break)
{
	static const char path[] = "../a/b/../../../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_dot)
{
	static const char path[] = "../a/./././b/./././c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_mixed_dot)
{
	static const char path[] = "../a/./././../b/././.././../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_dot)
{
	static const char path[] = "../a/./././../b/././.././../c/.";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_double_dot)
{
	static const char path[] = "../a/./././b/././../c/..";
	static const char result[] = "a";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_mixed_dot)
{
	static const char path[] = "../a/b/c/.././../.";
	static const char result[] = "a";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_mixed_dot)
{
	static const char path[] = "/a/b/c/../../../.";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_dot)
{
	static const char path[] = "/.";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_relative_begin_end_dot)
{
	static const char path[] = "./.";
	static const char result[] = ".";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_many_dot)
{
	static const char path[] = "/./././";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_STREQ(canonical, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_compare_root_equal)
{
	static const char left[] = "/";
	static const char right[] = "/";

	EXPECT_EQ((int)0, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_compare_equal)
{
	static const char left[] = "/a/b/c";
	static const char right[] = "/a/b/c";

	EXPECT_EQ((int)0, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_compare_equal_trailing_slash_inverted)
{
	static const char left[] = "/a/b/c/";
	static const char right[] = "/a/b/c";

	EXPECT_EQ((int)0, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_compare_equal_trailing_slash)
{
	static const char left[] = "/a/b/c";
	static const char right[] = "/a/b/c/";

	EXPECT_EQ((int)0, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_compare_root_empty)
{
	static const char left[] = "/";
	static const char right[] = "";

	EXPECT_EQ((int)1, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_compare_root_file)
{
	static const char left[] = "/a";
	static const char right[] = "/";

	EXPECT_EQ((int)1, (int)portability_path_compare(left, right));
}

TEST_F(portability_path_test, portability_path_test_fullname)
{
	static const char exe_path[] = "/usr/bin/qemu-riscv64";
	char exe_name[PORTABILITY_PATH_SIZE];
	size_t exe_name_size = portability_path_get_fullname(exe_path, sizeof(exe_path), exe_name, PORTABILITY_PATH_SIZE);

	EXPECT_NE((int)0, (int)exe_name_size);

	EXPECT_STREQ(exe_name, "qemu-riscv64");
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_basic)
{
	static const char base[] = "/a/b/c/foo.bar.baz";
	static const char result[] = "foo";

	string_name name;

	size_t size = portability_path_get_name_canonical(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_single_ext)
{
	static const char base[] = "/a/b/c/foo.txt";
	static const char result[] = "foo";

	string_name name;

	size_t size = portability_path_get_name_canonical(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_no_ext)
{
	static const char base[] = "/a/b/c/foo";
	static const char result[] = "foo";

	string_name name;

	size_t size = portability_path_get_name_canonical(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_dotfile)
{
	static const char base[] = "/a/b/c/.hidden";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name_canonical(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_dotfile_with_ext)
{
	static const char base[] = "/a/b/c/.hidden.txt";
	static const char result[] = ".hidden";

	string_name name;

	size_t size = portability_path_get_name_canonical(base, sizeof(base), name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_null_path)
{
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name_canonical(nullptr, 0, name, NAME_SIZE);

	EXPECT_STREQ(name, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)name[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_name_canonical_null_buffer_query)
{
	static const char base[] = "/a/b/c/foo.txt";

	size_t size = portability_path_get_name_canonical(base, sizeof(base), nullptr, NAME_SIZE);

	EXPECT_EQ((size_t)size, (size_t)4);
}

TEST_F(portability_path_test, portability_path_test_get_extension_basic)
{
	static const char base[] = "/a/b/c/file.txt";
	static const char result[] = "txt";

	string_name extension;

	size_t size = portability_path_get_extension(base, sizeof(base), extension, NAME_SIZE);

	EXPECT_STREQ(extension, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)extension[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_extension_double)
{
	static const char base[] = "/a/b/c/file.tar.gz";
	static const char result[] = "gz";

	string_name extension;

	size_t size = portability_path_get_extension(base, sizeof(base), extension, NAME_SIZE);

	EXPECT_STREQ(extension, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)extension[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_extension_no_ext)
{
	static const char base[] = "/a/b/c/file";
	// Rationale: Separator boundaries reset the extension search, consistent with
	// per-basename-component semantics. A file with no dot has no extension.
	static const char result[] = "";

	string_name extension;

	size_t size = portability_path_get_extension(base, sizeof(base), extension, NAME_SIZE);

	EXPECT_STREQ(extension, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)extension[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_extension_dotfile)
{
	static const char base[] = "/a/b/c/.hidden";
	static const char result[] = "hidden";

	string_name extension;

	size_t size = portability_path_get_extension(base, sizeof(base), extension, NAME_SIZE);

	EXPECT_STREQ(extension, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)extension[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_extension_trailing_dot)
{
	static const char base[] = "/a/b/c/file.";
	static const char result[] = "";

	string_name extension;

	size_t size = portability_path_get_extension(base, sizeof(base), extension, NAME_SIZE);

	EXPECT_STREQ(extension, result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)extension[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_extension_null)
{
	static const char base[] = "/a/b/c/file.txt";

	size_t size = portability_path_get_extension(base, sizeof(base), nullptr, NAME_SIZE);

	// Note: The previous behavior (0) was an inconsistency with sibling functions
	// (e.g. get_name_canonical). This has been fixed to correctly return the
	// required size for the query contract.
	EXPECT_EQ((size_t)size, (size_t)4);
}

TEST_F(portability_path_test, portability_path_test_get_extension_undersized_buffer)
{
	static const char base[] = "/a/b/c/file.txt";
	string_name extension = "garbage";

	size_t size = portability_path_get_extension(base, sizeof(base), extension, 2);

	// When the buffer is too small, the function must return the required
	// size without touching the buffer contents.
	EXPECT_EQ((size_t)size, (size_t)4);
	EXPECT_STREQ(extension, "garbage");
}

TEST_F(portability_path_test, portability_path_test_get_extension_truncated)
{
	static const char base[] = "/a/b/c/.hidden";
	string_name extension;

	// Pass path_size = 7, which truncates the string exactly at the last separator: "/a/b/c/"
	// This exercises the edge case where the loop terminates exactly on a separator boundary,
	// checking that ext_start == NULL falls back safely to path + i without underflowing.
	size_t size = portability_path_get_extension(base, 7, extension, NAME_SIZE);

	EXPECT_EQ((size_t)size, (size_t)1);
	EXPECT_STREQ(extension, "");
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_basic)
{
	std::string path = "/a/b/c/foo.txt";
	static const char result[] = "/a/b/c/";

	size_t size = portability_path_get_directory_inplace(path.data(), path.length() + 1);

	EXPECT_STREQ(path.data(), result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)path.data()[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_trailing_slash)
{
	std::string path = "/a/b/c/";
	static const char result[] = "/a/b/c/";

	size_t size = portability_path_get_directory_inplace(path.data(), path.length() + 1);

	EXPECT_STREQ(path.data(), result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)path.data()[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_no_slash)
{
	std::string path = "foo.txt";
	static const char result[] = "";

	size_t size = portability_path_get_directory_inplace(path.data(), path.length() + 1);

	EXPECT_STREQ(path.data(), result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)path.data()[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_root)
{
	std::string path = "/";
	static const char result[] = "/";

	size_t size = portability_path_get_directory_inplace(path.data(), path.length() + 1);

	EXPECT_STREQ(path.data(), result);
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)path.data()[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_null)
{
	size_t size = portability_path_get_directory_inplace(nullptr, NAME_SIZE);

	EXPECT_EQ((size_t)size, (size_t)0);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_zero_size)
{
	std::array<char, 1> path = { 'a' }; // Valid only if size > 0, but we pass size = 0

	size_t size = portability_path_get_directory_inplace(path.data(), 0);

	// The function should return 0 when size is 0 and leave buffer completely untouched.
	EXPECT_EQ((size_t)size, (size_t)0);
	EXPECT_EQ((char)'a', path[0]);
}

TEST_F(portability_path_test, portability_path_test_get_directory_inplace_undersized)
{
	std::array<char, 3> path = { '/', 'a', '/' }; // Not null-terminated, exactly size 3

	size_t size = portability_path_get_directory_inplace(path.data(), 3);

	// For an undersized buffer, the implementation must safely cap the null terminator
	// at the end of the buffer (path[size-1]) to prevent out-of-bounds writes.
	EXPECT_EQ((size_t)size, (size_t)3);
	EXPECT_EQ((char)'\0', path[2]);
}

#if defined(WIN32) || defined(_WIN32)

TEST_F(portability_path_test, portability_path_test_is_absolute_true)
{
	static const char path[] = "C:\\a\\b";
	EXPECT_EQ(0, portability_path_is_absolute(path, sizeof(path)));
}

TEST_F(portability_path_test, portability_path_test_is_absolute_false)
{
	static const char path[] = "\\a\\b";
	EXPECT_EQ(1, portability_path_is_absolute(path, sizeof(path)));
}

TEST_F(portability_path_test, portability_path_test_is_absolute_windows_lowercase)
{
	static const char path[] = "c:\\a\\b"; // Lowercase drive letter
	// Verify that lowercase drive letters are correctly identified as absolute paths on Windows.
	EXPECT_EQ(0, portability_path_is_absolute(path, sizeof(path)));
}

#else

TEST_F(portability_path_test, portability_path_test_is_absolute_true)
{
	static const char path[] = "/a/b";
	EXPECT_EQ(0, portability_path_is_absolute(path, sizeof(path)));
}

TEST_F(portability_path_test, portability_path_test_is_absolute_false)
{
	static const char path[] = "a/b";
	EXPECT_EQ(1, portability_path_is_absolute(path, sizeof(path)));
}

#endif

TEST_F(portability_path_test, portability_path_test_is_absolute_empty)
{
	static const char path[] = "";
	EXPECT_EQ(1, portability_path_is_absolute(path, sizeof(path)));
}

TEST_F(portability_path_test, portability_path_test_is_absolute_null)
{
	EXPECT_EQ(1, portability_path_is_absolute(nullptr, 10));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_exact)
{
	static const char parent[] = "/a/b";
	static const char child[] = "/a/b";
	EXPECT_EQ(0, portability_path_is_subpath(parent, 4, child, 4));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_child_longer)
{
	static const char parent[] = "/a/b";
	static const char child[] = "/a/b/c";
	// Verify that a valid child path is correctly identified as a subpath of the parent.
	EXPECT_EQ(0, portability_path_is_subpath(parent, sizeof(parent) - 1, child, sizeof(child) - 1));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_parent_longer)
{
	static const char parent[] = "/a/b/c";
	static const char child[] = "/a/b";
	// Ensure that a parent path is not incorrectly identified as a subpath of a shorter child.
	EXPECT_EQ(1, portability_path_is_subpath(parent, sizeof(parent) - 1, child, sizeof(child) - 1));
}

// Test that subpath checking handles non-null-terminated buffers safely without reading out of bounds.
TEST_F(portability_path_test, portability_path_test_is_subpath_out_of_bounds_read)
{
	static const char parent[] = "/a/b/c";
	std::array<char, 4> child = { '/', 'a', '/', 'b' }; // Not null terminated, exactly size 4
	EXPECT_EQ(1, portability_path_is_subpath(parent, sizeof(parent) - 1, child.data(), 4));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_different)
{
	static const char parent[] = "/a/b";
	static const char child[] = "/x/y";
	EXPECT_EQ(1, portability_path_is_subpath(parent, 4, child, 4));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_null)
{
	// Verify that passing NULL arguments safely returns 1 (false) without crashing.
	EXPECT_EQ(1, portability_path_is_subpath(nullptr, 0, nullptr, 0));
	EXPECT_EQ(1, portability_path_is_subpath("/a/b", 4, nullptr, 0));
}

TEST_F(portability_path_test, portability_path_test_is_subpath_partial_name)
{
	static const char parent[] = "/a/bc";
	static const char child[] = "/a/b";
	EXPECT_EQ(1, portability_path_is_subpath(parent, 5, child, 4));
}

TEST_F(portability_path_test, portability_path_test_separator_normalize_inplace_basic)
{
	std::string path = "C:\\a\\b";
	static const char expected[] = "C:/a/b";

	// Verify that the very first separator in a path is correctly normalized.
	EXPECT_EQ(0, portability_path_separator_normalize_inplace(path.data(), path.length() + 1));
	EXPECT_STREQ(expected, path.data());
}

TEST_F(portability_path_test, portability_path_test_separator_normalize_inplace_mixed)
{
	std::string path = "\\a/b\\c/";
	static const char expected[] = "/a/b/c/";

	EXPECT_EQ(0, portability_path_separator_normalize_inplace(path.data(), path.length() + 1));
	EXPECT_STREQ(expected, path.data());
}

TEST_F(portability_path_test, portability_path_test_separator_normalize_inplace_null)
{
	EXPECT_EQ(1, portability_path_separator_normalize_inplace(nullptr, 10));
}

TEST_F(portability_path_test, portability_path_test_is_pattern_star)
{
	EXPECT_EQ(0, portability_path_is_pattern("a*b", 3));
}

TEST_F(portability_path_test, portability_path_test_is_pattern_question)
{
	// Verify that the '?' character is correctly identified as a pattern wildcard.
	EXPECT_EQ(0, portability_path_is_pattern("a?b", 3));
}

TEST_F(portability_path_test, portability_path_test_exists_null)
{
	EXPECT_EQ(1, portability_path_exists(nullptr));
}

TEST_F(portability_path_test, portability_path_test_resolve_null)
{
	std::array<char, PORTABILITY_PATH_SIZE> resolved;
	resolved.fill('X'); // Sentinel to detect unwanted writes
	EXPECT_EQ((char *)nullptr, portability_path_resolve(nullptr, resolved.data()));

	// Check sentinel wasn't overwritten
	EXPECT_EQ('X', resolved[0]);
}

TEST_F(portability_path_test, portability_path_test_file_exists_null)
{
	EXPECT_EQ(1, portability_path_file_exists(nullptr));
}
