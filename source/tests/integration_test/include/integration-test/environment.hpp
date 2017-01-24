/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

class environment : public  testing::Environment
{
protected:
	loader_naming_name cs_script_name = "hello.cs";
	loader_naming_name py_script_name = "example.py";

public:
	void SetUp();

	void TearDown();
};
