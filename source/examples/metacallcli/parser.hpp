/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_PARSER_HPP
#define METACALL_CLI_PARSER_HPP 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include "tokenizer.hpp"

#include <iostream>

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

	/**
	*  @brief
	*    Assignement operator for parser
	*
	*  @return
	*    Returns a reference to itself
	*/
	parser & operator=(const parser &) = delete;

  protected:

	/* -- Private Member Data -- */

	const tokenizer::iterator & it;	/**< Tokenizer iterator reference */
};

/**
*  @brief
*    Custom parser utility for parameters
*/
class parser_parameter : public parser
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
	parser_parameter(const tokenizer::iterator & it);

	/**
	*  @brief
	*    Parser class destructor
	*/
	~parser_parameter(void);

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

	/**
	*  @brief
	*    Assignement operator for parser
	*
	*  @return
	*    Returns a reference to itself
	*/
	parser_parameter & operator=(const parser_parameter &) = delete;
};

/**
*  @brief
*    Check if current token is a custom quoted char ('')
*
*  @return
*    True if current token is a custom quoted char
*/
template <> bool parser_parameter::is<char>(void);

/**
*  @brief
*    Check if current token is a custom long ending with (L)
*
*  @return
*    True if current token is a custom long
*/
template <> bool parser_parameter::is<long>(void);

/**
*  @brief
*    Check if current token is a custom quoted string ("")
*
*  @return
*    True if current token is a custom quoted string
*/
template <> bool parser_parameter::is<std::string>(void);

/**
*  @brief
*    Return current token trnasformed from custom quoted char ('')
*
*  @return
*    A copy of the value transformed to char
*/
template <> char parser_parameter::to<char>(void);

/**
*  @brief
*    Return current token trnasformed from custom long ending with (L)
*
*  @return
*    A copy of the value transformed to long
*/
template <> long parser_parameter::to(void);

/**
*  @brief
*    Return current token trnasformed from custom quoted string ('')
*
*  @return
*    A copy of the value transformed to string
*/
template <> std::string parser_parameter::to(void);

} /* namespace metacallcli */

/* -- Template Implementation -- */

#include "parser.inl"

#endif /* METACALL_CLI_PARSER_HPP */
