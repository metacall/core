/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_PACKAGE_MANAGER_PIP_HPP
#define METACALL_CLI_PACKAGE_MANAGER_PIP_HPP 1

/* -- Namespace -- */

namespace metacallcli {

namespace package_manager {

static const char pip[] =
"#!/usr/bin/env python3\n"
"\n"
"try:\n"
"	from pip import main as pipmain\n"
"except ImportError:\n"
"	from pip._internal import main as pipmain\n"
"\n"
"def package_manager(args):\n"
"	return pipmain(args);\n";

} /* namespace package_manager */

} /* namespace metacallcli */

#endif /* METACALL_CLI_PACKAGE_MANAGER_PIP_HPP */
