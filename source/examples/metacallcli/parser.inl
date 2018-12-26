/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <string>
#include <sstream>

/* -- Namespace -- */

namespace metacallcli {

/* -- Methods -- */

template <typename T> bool parser::is()
{
	std::stringstream str_stream(*it);

	T t;

	return (str_stream >> t && !str_stream.ignore());
}

template <typename T> T parser::to()
{
	std::stringstream str_stream(*it);

	T t;

	str_stream >> t;

	return t;
}

template <typename T> bool parser_parameter::is()
{
	std::stringstream str_stream(*it);

	T t;

	return (str_stream >> std::noskipws >> t && !str_stream.ignore());
}

template <typename T> T parser_parameter::to()
{
	std::stringstream str_stream(*it);

	T t;

	str_stream >> std::noskipws >> t;

	return t;
}

} /* namespace metacallcli */
