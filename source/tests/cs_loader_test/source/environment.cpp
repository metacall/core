/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */
#include <cs-loader-test/environment.hpp>

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <log/log.h>

void environment::SetUp() {

	loader_naming_path names[] = {"hello.cs","IJump.cs","JumpMaster.cs","SuperJump.cs","TinyJump.cs"};

	loader_load_from_file(names, 5);
}

void environment::TearDown() {
	loader_unload();
}
