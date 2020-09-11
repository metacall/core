/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include <metacallcli/parser.hpp>

#include <iostream>

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Methods -- */

parser::parser(const tokenizer::iterator & it) : it(it)
{

}

parser::~parser()
{

}

parser_parameter::parser_parameter(const tokenizer::iterator & it) : parser(it)
{

}

parser_parameter::~parser_parameter()
{

}

template <> bool parser_parameter::is<char>()
{
	const std::string & str = *it;

	size_t len = str.length();

	char first = str[0];

	char last = str[len - 1];

	return (first == last) && (first == '\'');
}

template <> bool parser_parameter::is<long>()
{
	const std::string & str = *it;

	size_t length = str.length();

	std::stringstream str_stream;

	long l;

	if (isdigit(str[0]) && str[length - 1] == 'L')
	{
		str_stream.str(str.substr(0, length - 1));
	}
	else
	{
		str_stream.str(str);
	}

	return (str_stream >> std::noskipws >> l && !str_stream.ignore());
}

template <> bool parser_parameter::is<float>()
{
	const std::string & str = *it;

	size_t length = str.length();

	std::stringstream str_stream;

	float f;

	if (str[length - 1] == 'f')
	{
		str_stream.str(str.substr(0, length - 1));
	}
	else
	{
		str_stream.str(str);
	}

	return (str_stream >> std::noskipws >> f && !str_stream.ignore());
}

template <> bool parser_parameter::is<std::string>()
{
	const std::string & str = *it;

	size_t length = str.length();

	char first = str[0];

	char last = str[length - 1];

	return (first == last) && (first == '"');
}

template <> char parser_parameter::to<char>()
{
	const std::string & str = *it;

	size_t length = str.length();

	char c = str[1];

	if (length < 4)
	{
		return c;
	}

	if (length < 5 && c == '\\')
	{
		char special = str[2];

		switch (special)
		{
			case 't' : return '\t';
			case 'n' : return '\n';
			case 'r' : return '\r';
			case 'v' : return '\v';
			case 'f' : return '\f';
			case '0' : return '\0';
			case '\\' : return '\\';
			case '\'' : return '\'';

			default:
			{
				std::cout << "Invalid character (" << special << ")" << std::endl;

				return special;
			}
		}
	}

	return '\0';
}

template <> long parser_parameter::to<long>()
{
	const std::string & str = *it;

	size_t length = str.length();

	std::stringstream str_stream;

	long l;

	if (isdigit(str[0]) && str[length - 1] == 'L')
	{
		str_stream.str(str.substr(0, length - 1));
	}
	else
	{
		str_stream.str(str);
	}

	str_stream >> std::noskipws >> l;

	return l;
}

template <> float parser_parameter::to<float>()
{
	const std::string & str = *it;

	size_t length = str.length();

	std::stringstream str_stream;

	float f;

	if (str[length - 1] == 'f')
	{
		str_stream.str(str.substr(0, length - 1));
	}
	else
	{
		str_stream.str(str);
	}

	str_stream >> std::noskipws >> f;

	return f;
}

template <> std::string parser_parameter::to<std::string>()
{
	const std::string & str = *it;

	size_t length = str.length() - 2;

	return str.substr(1, length);
}
