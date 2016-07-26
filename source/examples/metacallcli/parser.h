/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_PARSER_H
#define METACALL_CLI_PARSER_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include "tokenizer.h"

/* -- Namespace -- */

namespace metacallcli {

/* -- Forward Declarations -- */

class tokenizer;

class parser;

/* -- Class Definition -- */

/**
*  @brief
*    Simple parser utility
*/
class parser
{
  public:

	/* -- Public Methods -- */

	/**
	*  @brief
	*    Parser class constructor
	*
	*  @param[in] it
	*    Tokenizer iterator reference
	*/
	parser(const tokenizer::iterator & it);

	/**
	*  @brief
	*    Parser class destructor
	*/
	~parser(void);

	/**
	*  @brief
	*    Check if current token is a defined type @T
	*
	*  @param[template] T
	*    Type of value to check against token
	*
	*  @return
	*    True if type @T is equivalent to current token
	*/
	template <typename T> bool is(void);

	/**
	*  @brief
	*    Return current token trnasformed to type @T
	*
	*  @param[template] T
	*    Type of value to check against token
	*
	*  @return
	*    A copy of the value transformed to given type @T
	*/
	template <typename T> T to(void);

  private:

	/* -- Private Member Data -- */

	const tokenizer::iterator & it;	/**< Tokenizer iterator reference */
};

} /* namespace metacallcli */

#include "parser.inl"

#endif /* METACALL_CLI_PARSER_H */
