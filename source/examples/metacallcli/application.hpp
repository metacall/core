/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_APPLICATION_HPP
#define METACALL_CLI_APPLICATION_HPP 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#include <reflect/reflect_value_type.h>

#include <string>
#include <vector>
#include <unordered_map>

/* -- Namespace -- */

namespace metacallcli {

/* -- Forward Declarations -- */

class tokenizer;

class parser;
class parser_parameter;

class application;

/* -- Class Definition -- */

/**
*  @brief
*    MetaCall command line interface applicaton
*/
class application
{
  public:

	/* -- Public Type Definitions -- */

	typedef bool (*command_callback)(application &, tokenizer &);

	/* -- Public Methods -- */

	/**
	*  @brief
	*    Application class constructor
	*
	*  @param[in] argc
	*    Number of elements contained in program parameters
	*
	*  @param[in] argv
	*    Array of strings from program parameters
	*/
	application(int argc, char * argv[]);

	/**
	*  @brief
	*    Application class destructor
	*/
	~application(void);

	/**
	*  @brief
	*    Application script loader
	*
	*  @param[in] script
	*    Reference to script name
	*
	*  @return
	*    Return true on success, false otherwhise
	*/
	bool load(const std::string & script);

	/**
	*  @brief
	*    Application usage information
	*/
	void usage(void);

	/**
	*  @brief
	*    Application main entry point
	*/
	void run(void);

	/**
	*  @brief
	*    Shutdown main application loop
	*/
	void shutdown(void);

	/**
	*  @brief
	*    Debug command line string
	*
	*  @param[in] key
	*    Name of the command line option
	*
	*  @param[in] t
	*    Tokenizer wrapper of input command
	*/
	void command_debug(const std::string & key, const tokenizer & t);

	/**
	*  @brief
	*    Create a new value from arguments with parser @p
	*
	*  @param[in] p
	*    Parser which points to the current iterator of the string
	*
	*  @return
	*    Return a new value instanced if argument was correct
	*/
	value argument_parse(parser_parameter & p);

	/**
	*  @brief
	*    Adapts metacallv from string @name and vector @args
	*
	*  @param[in] name
	*    String object of function name
	*
	*  @param[in] args
	*    Vector pointing to arguments
	*
	*  @return
	*    Return a new value instanced if argument was correct with the result of the call
	*/
	value metacallv_adaptor(const std::string & name, const std::vector<value> & args);

  protected:

	/* -- Protected Definitions -- */

	static const size_t arguments_str_size;

	/* -- Protected Methods -- */

	/**
	*  @brief
	*    Execute a command with string parameters
	*
	*  @param[in out] t
	*    Tokenizer wrapper of input command
	*/
	void execute(tokenizer & t);

	/**
	*  @brief
	*    Defines a new command with a callback handler
	*
	*  @param[in] key
	*    Name of the command line option
	*
	*  @param[in] command_cb
	*    Handler will be raised on @key command entered
	*/
	void define(const char * key, command_callback command_cb);

  private:

	/* -- Private Type Definitions -- */

	typedef std::vector<std::string> script_list;

	typedef std::unordered_map<std::string, command_callback> command_table;

	/* -- Private Class Definition -- */

	class parameter_iterator
	{
	  public:

		/* -- Public Methods -- */

		/**
		*  @brief
		*    Initialize parameter iterator
		*
		*  @param[in] app
		*    Reference to the application
		*
		*  @param[in] scripts
		*    Reference to list of scripts to be iterated
		*/
		parameter_iterator(application & app, script_list & scripts);

		/**
		*  @brief
		*    Parameter iterator class destructor
		*/
		~parameter_iterator();

		/**
		*  @brief
		*    Operator callback for iteration
		*
		*  @param[in] parameter
		*    Current parameter being iterated
		*/
		void operator()(const char * parameter);

		/**
		*  @brief
		*    Assignement operator for parameter iterator
		*
		*  @return
		*    Returns a reference to itself
		*/
		parameter_iterator & operator=(const parameter_iterator &) = delete;

	  private:

		/* -- Private Member Data -- */

		application & app;	/**< Reference to the application */

		script_list & scripts;	/**< Reference to the script list */
	};

	/* -- Private Member Data -- */

	bool exit_condition;	/**< Condition for main loop */

	script_list scripts;	/**< Vector containing a list of script names */

	command_table commands;	/**< Hash table from command strings to command handlers */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_APPLICATION_HPP */
