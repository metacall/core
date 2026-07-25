#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_rust_template_test : public testing::Test
{
public:
};

TEST_F(metacall_rust_template_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ(0, metacall_initialize());

	const char script[] =
		"pub fn identity<T>(x: T) -> T {"
		"	x"
		"}";

	int result = metacall_load_from_memory(
		"rs",
		script,
		sizeof(script) - 1,
		NULL);

	ASSERT_EQ(0, result);

	void *f = metacall_function("identity");

	std::cout << "function ptr: " << f << std::endl;

	metacall_destroy();
}