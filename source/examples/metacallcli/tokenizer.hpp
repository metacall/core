/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_TOKENIZER_HPP
#define METACALL_CLI_TOKENIZER_HPP 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include <string>
#include <iterator>
#include <vector>

/* -- Namespace -- */

namespace metacallcli {

/* -- Forward Declarations -- */

class tokenizer;

/* -- Class Definition -- */

/**
*  @brief
*    String tokenizer utility
*/
class tokenizer
{
  public:

	/* -- Public Methods -- */

	/**
	*  @brief
	*    Tokenizer constructor with default delimiters
	*
	*  @param[in] str
	*    String to be tokenized
	*/
	tokenizer(const std::string & str);

	/**
	*  @brief
	*    Tokenizer constructor with custom @delimiters
	*
	*  @param[in] str
	*    String to be tokenized
	*
	*  @param[in] delimiters
	*    Delimiters will be used to tokenize the string @str
	*/
	tokenizer(const std::string & str, const std::string & delimiters);

	/**
	*  @brief
	*    Tokenizer class destructor
	*/
	~tokenizer(void);

	/**
	*  @brief
	*    Set custom tokenizer delimiters
	*
	*  @param[in] del
	*    Delimiters will be used to tokenize the string
	*/
	void delimit(const std::string & del);

	/* -- Public Class Definition -- */

	/**
	*  @brief
	*    String tokenizer iterator
	*/
	class iterator : public std::iterator<
				std::input_iterator_tag,
				size_t,
				size_t,
				const size_t *,
				const std::string &>
	{
	  public:

		/* -- Public Methods -- */

		/**
		*  @brief
		*    Tokenizer iterator constructor
		*
		*  @param[in] t
		*    Reference to tokenizer
		*
		*  @param[in] begin
		*    Initial iterator position
		*/
		explicit iterator(const tokenizer & t, size_t begin = 0);

		/**
		*  @brief
		*    Tokenizer iterator increment operator
		*
		*  @return
		*    Return a reference to itself
		*/
		iterator & operator++(void);

		/**
		*  @brief
		*    Tokenizer iterator increment operator
		*
		*  @return
		*    Return a copy of itself
		*/
		iterator operator++(int);

		/**
		*  @brief
		*    Tokenizer iterator equality operator
		*
		*  @param[in] other
		*    Reference to iterator to be compared against
		*
		*  @return
		*    Return true if @other iterator points to the same @str and @offset
		*/
		bool operator==(iterator other) const;

		/**
		*  @brief
		*    Tokenizer iterator inequality operator
		*
		*  @param[in] other
		*    Reference to iterator to be compared against
		*
		*  @return
		*    Return true if @other iterator points to different @str or @offset
		*/
		bool operator!=(iterator other) const;

		/**
		*  @brief
		*    Tokenizer iterator dereferencing operator
		*
		*  @return
		*    Return a constant string reference to the current token
		*/
		reference operator*(void) const;

		/**
		*  @brief
		*    Tokenizer iterator current position
		*
		*  @return
		*    Return current value of @offset
		*/
		size_t position(void) const;

		/**
		*  @brief
		*    Tokenizer iterator escape trailing characters
		*
		*  @param[in] characters
		*    Reference to list of characters to be escaped
		*
		*  @return
		*    Return a constant string reference to the current token
		*/
		reference escape(const std::string & characters);

		/**
		*  @brief
		*    Assignement operator for tokenizer iterator
		*
		*  @return
		*    Returns a reference to itself
		*/
		iterator & operator=(const iterator &) = delete;

	  private:

		/* -- Private Member Data -- */

		const tokenizer & t;			/**< Reference to tokenizer */

		size_t offset;				/**< Current position over iteration */

		std::string token;			/**< Current token */
	};

	/**
	*  @brief
	*    Begin iterator operation
	*
	*  @return
	*    Returns a iterator pointing to the beginning of @str
	*/
	iterator begin(void) const;

	/**
	*  @brief
	*    End iterator operation
	*
	*  @return
	*    Returns a iterator pointing to the end of @str
	*/
	iterator end(void) const;

	/**
	*  @brief
	*    Assignement operator for tokenizer
	*
	*  @return
	*    Returns a reference to itself
	*/
	tokenizer & operator=(const tokenizer &) = delete;

  protected:

	/* -- Protected Definitions -- */

	static const std::string default_delimiters;

  private:

	/* -- Private Member Data -- */

	const std::string str;		/**< String to be tokenized */

	std::string delimiters;		/**< Current string token delimiters */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_TOKENIZER_HPP */
