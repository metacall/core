/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

/* -- Headers -- */

#include "tokenizer.hpp"

/* -- Namespace Declarations -- */

using namespace metacallcli;

/* -- Protected Definitions -- */

const std::string tokenizer::default_delimiters(" \n\t\r\v\f");

/* -- Methods -- */

tokenizer::tokenizer(const std::string & str) : str(str), delimiters(default_delimiters)
{

}

tokenizer::tokenizer(const std::string & str, const std::string & delimiters) :
	str(str), delimiters(delimiters)
{

}

tokenizer::~tokenizer()
{

}

tokenizer::iterator::iterator(const tokenizer & t, size_t begin) :
	t(t), offset(begin), token("")
{
	++(*this);
}

void tokenizer::delimit(const std::string & del)
{
	delimiters = del;
}

tokenizer::iterator & tokenizer::iterator::operator++()
{
	size_t token_position = t.str.find_first_not_of(t.delimiters, offset);

	size_t next_delimiter_position;

	if (std::string::npos == token_position)
	{
		offset = t.str.length() + 1;

		return *this;
	}

	next_delimiter_position = t.str.find_first_of(t.delimiters, token_position);

	if (std::string::npos == next_delimiter_position)
	{
		token = t.str.substr(token_position);

		offset = t.str.length();

		return *this;
	}

	token = t.str.substr(token_position, next_delimiter_position - token_position);

	offset = next_delimiter_position;

	return *this;
}

tokenizer::iterator tokenizer::iterator::operator++(int)
{
	iterator ret = *this;

	++(*this);

	return ret;
}

bool tokenizer::iterator::operator==(iterator other) const
{
	return offset == other.offset && t.str == other.t.str;
}

bool tokenizer::iterator::operator!=(iterator other) const
{
	return !(*this == other);
}

tokenizer::iterator::reference tokenizer::iterator::operator*() const
{
	return token;
}

size_t tokenizer::iterator::position() const
{
	return offset;
}

tokenizer::iterator::reference tokenizer::iterator::escape(const std::string & characters)
{
	token.erase(0, token.find_first_not_of(characters));

	token.erase(token.find_last_not_of(characters) + 1);

	return token;
}

tokenizer::iterator tokenizer::begin() const
{
	return tokenizer::iterator(*this);
}

tokenizer::iterator tokenizer::end() const
{
	return tokenizer::iterator(*this, str.length() + 1);
}
