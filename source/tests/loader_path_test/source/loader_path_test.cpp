/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for loading executable code at run-time into a process.
*
*/

#include <gtest/gtest.h>

#include <loader/loader_path.h>

#include <cstring>

class loader_path_test : public testing::Test
{
public:
};

TEST_F(loader_path_test, loader_path_test_path_get_module_name)
{
	const char base[] = "/a/b/c/asd.txt";
	const char result[] = "asd";

	loader_naming_name name;

	size_t size = loader_path_get_module_name(base, name, "txt");

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_module_name_without_extension)
{
	const char base[] = "/a/b/c/asd";
	const char result[] = "asd";

	loader_naming_name name;

	size_t size = loader_path_get_module_name(base, name, "txt");

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_module_name_with_random_extension)
{
	const char base[] = "/a/b/c/asd.etc.asd";
	const char result[] = "asd.etc.asd";

	loader_naming_name name;

	size_t size = loader_path_get_module_name(base, name, "txt");

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name)
{
	const char base[] = "/a/b/c/asd.txt";
	const char result[] = "asd";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_end_dot)
{
	const char base[] = "/a/b/c/asd.";
	const char result[] = "asd";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_without_dot)
{
	const char base[] = "/a/b/c/asd";
	const char result[] = "asd";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_only_dot)
{
	const char base[] = "/a/b/c/.";
	const char result[] = "";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_two_dots)
{
	const char base[] = "/a/b/c/..";
	const char result[] = "";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_three_dots)
{
	const char base[] = "/a/b/c/...";
	const char result[] = "";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_only_extension)
{
	const char base[] = "/a/b/c/.asd";
	const char result[] = ".asd";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_double_extension)
{
	const char base[] = "/a/b/c/.asd.yes";
	const char result[] = ".asd";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_path_get_name_triple_extension)
{
	const char base[] = "/a/b/c/.asd.yes.no";
	const char result[] = ".asd.yes";

	loader_naming_name name;

	size_t size = loader_path_get_name(base, name);

	EXPECT_EQ((int)0, (int)strcmp(name, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_get_path_of_path)
{
	const char base[] = "/a/b/c/";
	const char result[] = "/a/b/c/";

	loader_naming_path path;

	size_t size = loader_path_get_path(base, sizeof(base), path);

	EXPECT_EQ((int)0, (int)strcmp(path, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_get_path_of_filepath)
{
	const char base[] = "/a/b/c/asd";
	const char result[] = "/a/b/c/";

	loader_naming_path path;

	size_t size = loader_path_get_path(base, sizeof(base), path);

	EXPECT_EQ((int)0, (int)strcmp(path, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_get_relative)
{
	const char base[] = "/a/b/c/";
	const char path[] = "/a/b/c/abc";
	const char result[] = "abc";

	loader_naming_path relative;

	size_t size = loader_path_get_relative(base, path, relative);

	EXPECT_EQ((int)0, (int)strcmp(relative, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_get_relative_fail)
{
	const char base[] = "/this/is/not/shared/with/path";
	const char path[] = "/a/b/c/abc";
	const char result[] = "a/b/c/abc";

	loader_naming_path relative;

	size_t size = loader_path_get_relative(base, path, relative);

	EXPECT_EQ((int)0, (int)strcmp(relative, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_none_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_left_slash)
{
	const char left[] = "/a/b/c/";
	const char right[] = "e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "/e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_both_slash)
{
	const char left[] = "/a/b/c/";
	const char right[] = "/e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_left_empty)
{
	const char left[] = "";
	const char right[] = "/e/f/g/";
	const char result[] = "/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_empty)
{
	const char left[] = "/a/b/c/";
	const char right[] = "";
	const char result[] = "/a/b/c/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_empty_non_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "";
	const char result[] = "/a/b/c/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_both_empty)
{
	const char left[] = "";
	const char right[] = "";
	const char result[] = "";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int)0, (int)strcmp(join, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_dot)
{
	const char path[] = "./a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_double_dot)
{
	const char path[] = "../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_dot)
{
	const char path[] = "./././././a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_double_dot)
{
	const char path[] = "../../../../../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_dot_non_slash)
{
	const char path[] = ".a/b/c";
	const char result[] = ".a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_dot_non_slash)
{
	const char path[] = "..a/b/c";
	const char result[] = "..a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_invalid)
{
	const char path[] = "..././.../...../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot)
{
	const char path[] = "../a/b/../c";
	const char result[] = "a/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot_all)
{
	const char path[] = "../a/b/../../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot_break)
{
	const char path[] = "../a/b/../../../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_dot)
{
	const char path[] = "../a/./././b/./././c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_mixed_dot)
{
	const char path[] = "../a/./././../b/././.././../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_dot)
{
	const char path[] = "../a/./././../b/././.././../c/.";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_double_dot)
{
	const char path[] = "../a/./././b/././../c/..";
	const char result[] = "a";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_mixed_dot)
{
	const char path[] = "../a/b/c/.././../.";
	const char result[] = "a";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_mixed_dot)
{
	const char path[] = "/a/b/c/../../../.";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_dot)
{
	const char path[] = "/.";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_relative_begin_end_dot)
{
	const char path[] = "./.";
	const char result[] = ".";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_many_dot)
{
	const char path[] = "/./././";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int)0, (int)strcmp(canonical, result));
	EXPECT_EQ((size_t)size, (size_t)sizeof(result));
	EXPECT_EQ((char)'\0', (char)result[size - 1]);
}
