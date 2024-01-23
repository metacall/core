/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for loading executable code at run-time into a process.
*
*/

#include <gtest/gtest.h>

#include <portability/portability_path.h>

#include <cstring>

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

	EXPECT_EQ((int)0, (int)strcmp(name, result));
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

	EXPECT_EQ((int)0, (int)strcmp(name, result));
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

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name)
{
	static const char base[] = "/a/b/c/asd.txt";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_end_dot)
{
	static const char base[] = "/a/b/c/asd.";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_without_dot)
{
	static const char base[] = "/a/b/c/asd";
	static const char result[] = "asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_only_dot)
{
	static const char base[] = "/a/b/c/.";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_two_dots)
{
	static const char base[] = "/a/b/c/..";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_three_dots)
{
	static const char base[] = "/a/b/c/...";
	static const char result[] = "";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_only_extension)
{
	static const char base[] = "/a/b/c/.asd";
	static const char result[] = ".asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_double_extension)
{
	static const char base[] = "/a/b/c/.asd.yes";
	static const char result[] = ".asd";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_path_get_name_triple_extension)
{
	static const char base[] = "/a/b/c/.asd.yes.no";
	static const char result[] = ".asd.yes";

	string_name name;

	size_t size = portability_path_get_name(base, sizeof(base), name, NAME_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_path_of_path)
{
	static const char base[] = "/a/b/c/";
	static const char result[] = "/a/b/c/";

	string_path path;

	size_t size = portability_path_get_directory(base, sizeof(base), path, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(path, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_get_path_of_filepath)
{
	static const char base[] = "/a/b/c/asd";
	static const char result[] = "/a/b/c/";

	string_path path;

	size_t size = portability_path_get_directory(base, sizeof(base), path, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(path, result));
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

	EXPECT_EQ((int)0, (int)strcmp(relative, result));
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

	EXPECT_EQ((int)0, (int)strcmp(relative, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
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

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_dot)
{
	static const char path[] = "./a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_double_dot)
{
	static const char path[] = "../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_dot)
{
	static const char path[] = "./././././a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_double_dot)
{
	static const char path[] = "../../../../../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_dot_non_slash)
{
	static const char path[] = ".a/b/c";
	static const char result[] = ".a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_many_dot_non_slash)
{
	static const char path[] = "..a/b/c";
	static const char result[] = "..a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_begin_invalid)
{
	static const char path[] = "..././.../...../a/b/c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot)
{
	static const char path[] = "../a/b/../c";
	static const char result[] = "a/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot_all)
{
	static const char path[] = "../a/b/../../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_double_dot_break)
{
	static const char path[] = "../a/b/../../../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_dot)
{
	static const char path[] = "../a/./././b/./././c";
	static const char result[] = "a/b/c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_middle_mixed_dot)
{
	static const char path[] = "../a/./././../b/././.././../c";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_dot)
{
	static const char path[] = "../a/./././../b/././.././../c/.";
	static const char result[] = "c";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_double_dot)
{
	static const char path[] = "../a/./././b/././../c/..";
	static const char result[] = "a";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_end_mixed_dot)
{
	static const char path[] = "../a/b/c/.././../.";
	static const char result[] = "a";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_mixed_dot)
{
	static const char path[] = "/a/b/c/../../../.";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_dot)
{
	static const char path[] = "/.";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_relative_begin_end_dot)
{
	static const char path[] = "./.";
	static const char result[] = ".";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(portability_path_test, portability_path_test_canonical_absolute_end_many_dot)
{
	static const char path[] = "/./././";
	static const char result[] = "/";

	string_path canonical;

	size_t size = portability_path_canonical(path, sizeof(path), canonical, PATH_SIZE);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
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
