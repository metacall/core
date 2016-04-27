
#include <gmock/gmock.h>

#include <metacall/metacall.h>

class metacall_test : public testing::Test
{
  public:
};

TEST_F(metacall_test, CheckSomeResults)
{
	metacall_print_info();

	EXPECT_EQ((unsigned int) 0, (unsigned int) 0);
	EXPECT_EQ((unsigned int) 3, (unsigned int) 3);

	// ...
}
