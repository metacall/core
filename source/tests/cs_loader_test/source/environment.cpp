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

	loader_naming_path names[] = {"hello.cs"};

	log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout));

	loader_load_from_files(names, 1);
}

void environment::TearDown() {
	loader_unload();
}
