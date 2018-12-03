/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

class environment : public  testing::Environment
{
public:
	void SetUp();

	void TearDown();
};
