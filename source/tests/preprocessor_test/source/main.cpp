/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

#include <gmock/gmock.h>

int main(int argc, char * argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	return RUN_ALL_TESTS();
}
