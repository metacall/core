/*
 *	MetaCall++ Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
 *
 *	High performance, type safe and exception safe object oriented
 *	front-end for MetaCall library.
 *
 */

#include <gmock/gmock.h>

int main(int argc, char * argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	return RUN_ALL_TESTS();
}
