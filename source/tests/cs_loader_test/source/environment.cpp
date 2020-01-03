/*
 *	MetaCall Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */
#include <cs-loader-test/environment.hpp>

#include <gmock/gmock.h>

#include <metacall/metacall.h>

void environment::SetUp()
{
	const char * cs_scripts[] =
	{
		"hello.cs",
		"IJump.cs",
		"JumpMaster.cs",
		"SuperJump.cs",
		"TinyJump.cs"
	};

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	EXPECT_EQ((int) 0, (int) metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));
}

void environment::TearDown()
{
	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
