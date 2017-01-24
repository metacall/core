/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>
#include <integration-test/environment.hpp>

int main(int argc, char * argv[])
{
	::testing::InitGoogleMock(&argc, argv);
	::testing::AddGlobalTestEnvironment(new environment());
	auto a = RUN_ALL_TESTS();
	getchar();
	return a;
}
