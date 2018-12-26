/*
*	Loader Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for loading executable code at run-time into a process.
*
*/

#include <gmock/gmock.h>

#include <loader/loader_path.h>

#include <cstring>

class loader_path_test : public testing::Test
{
  public:
};

TEST_F(loader_path_test, loader_path_test_get_path_of_path)
{
	const char base[] = "/a/b/c/";
	const char result[] = "/a/b/c/";

	loader_naming_path path;

	size_t size = loader_path_get_path(base, sizeof(base), path);

	EXPECT_EQ((int) 0, (int) strcmp(path, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_get_path_of_filepath)
{
	const char base[] = "/a/b/c/asd";
	const char result[] = "/a/b/c/";

	loader_naming_path path;

	size_t size = loader_path_get_path(base, sizeof(base), path);

	EXPECT_EQ((int)0, (int)strcmp(path, result));
	EXPECT_EQ((size_t)size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char)result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_none_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_left_slash)
{
	const char left[] = "/a/b/c/";
	const char right[] = "e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "/e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_both_slash)
{
	const char left[] = "/a/b/c/";
	const char right[] = "/e/f/g/";
	const char result[] = "/a/b/c/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_left_empty)
{
	const char left[] = "";
	const char right[] = "/e/f/g/";
	const char result[] = "/e/f/g/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_empty)
{
	const char left[] = "/a/b/c/";
	const char right[] = "";
	const char result[] = "/a/b/c/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_right_empty_non_slash)
{
	const char left[] = "/a/b/c";
	const char right[] = "";
	const char result[] = "/a/b/c/";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_join_both_empty)
{
	const char left[] = "";
	const char right[] = "";
	const char result[] = "";

	loader_naming_path join;

	size_t size = loader_path_join(left, sizeof(left), right, sizeof(right), join);

	EXPECT_EQ((int) 0, (int) strcmp(join, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_dot)
{
	const char path[] = "./a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_double_dot)
{
	const char path[] = "../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_dot)
{
	const char path[] = "./././././a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_double_dot)
{
	const char path[] = "../../../../../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_dot_non_slash)
{
	const char path[] = ".a/b/c";
	const char result[] = ".a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_many_dot_non_slash)
{
	const char path[] = "..a/b/c";
	const char result[] = "..a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_begin_invalid)
{
	const char path[] = "..././.../...../a/b/c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot)
{
	const char path[] = "../a/b/../c";
	const char result[] = "a/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot_all)
{
	const char path[] = "../a/b/../../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_double_dot_break)
{
	const char path[] = "../a/b/../../../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_dot)
{
	const char path[] = "../a/./././b/./././c";
	const char result[] = "a/b/c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_middle_mixed_dot)
{
	const char path[] = "../a/./././../b/././.././../c";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_dot)
{
	const char path[] = "../a/./././../b/././.././../c/.";
	const char result[] = "c";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_double_dot)
{
	const char path[] = "../a/./././b/././../c/..";
	const char result[] = "a";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_end_mixed_dot)
{
	const char path[] = "../a/b/c/.././../.";
	const char result[] = "a";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_mixed_dot)
{
	const char path[] = "/a/b/c/../../../.";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_dot)
{
	const char path[] = "/.";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_relative_begin_end_dot)
{
	const char path[] = "./.";
	const char result[] = ".";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}

TEST_F(loader_path_test, loader_path_test_canonical_absolute_end_many_dot)
{
	const char path[] = "/./././";
	const char result[] = "/";

	loader_naming_path canonical;

	size_t size = loader_path_canonical(path, sizeof(path), canonical);

	EXPECT_EQ((int) 0, (int) strcmp(canonical, result));
	EXPECT_EQ((size_t) size, (size_t) sizeof(result));
	EXPECT_EQ((char) '\0', (char) result[size - 1]);
}
