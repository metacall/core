/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <sstream>

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Methods -- */

template <typename T> bool parser::is()
{
	std::stringstream str_stream(*it);

	T value;

	return str_stream >> value && !str_stream.ignore();
}

template <typename T> T parser::to()
{
	std::stringstream str_stream(*it);

	T value;

	str_stream >> value;

	return value;
}
