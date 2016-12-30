/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <loader/loader.h>

#include <metacall/metacall.h>

#include <log/log.h>

class cs_loader_test : public testing::Test
{
protected:
};

TEST_F(cs_loader_test, SayHello)
{
	metacall("SayHello");
}

TEST_F(cs_loader_test, SayAny)
{
	metacall("Say","Any");
}

