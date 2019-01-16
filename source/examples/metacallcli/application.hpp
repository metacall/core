/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_APPLICATION_HPP
#define METACALL_CLI_APPLICATION_HPP 1

/* -- Headers -- */

#include <metacall/metacall.h>

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
	*  @param[in] tag
	*    Loader tag reference
	*
	*  @param[in] script
	*    Reference to script name
	*
	*  @return
	*    Return true on success, false otherwhise
	*/
	bool load(const std::string & tag, const std::string & script);

	/**
	*  @brief
	*    Application script clearer
	*
	*  @param[in] tag
	*    Loader tag reference
	*
	*  @param[in] script
	*    Reference to script name
	*
	*  @return
	*    Return true on success, false otherwhise
	*/
	bool clear(const std::string & tag, const std::string & script);

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
	*    Show inspect information
	*
	*  @param[in] str
	*    Serialized inspect data
	*
	*  @param[in] size
	*    Size in bytes of str string
	*
	*  @param[in] size
	*    Size in bytes of str string
	*
	*  @param[in] allocator
	*    Pointer to the allocator to be used in deserialization
	*/
	void command_inspect(const char * str, size_t size, void * allocator);

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
	void * argument_parse(parser_parameter & p);

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
	void * metacallv_adaptor(const std::string & name, const std::vector<void *> & args);

	/**
	*  @brief
	*    Adapts metacallfs from string @name and array string @args
	*
	*  @param[in] name
	*    String object of function name
	*
	*  @param[in] args
	*    String representing an array to be deserialized
	*
	*  @param[in] allocator
	*    Pointer to the allocator to be used in deserialization
	*
	*  @return
	*    Return a new value instanced if argument was correct with the result of the call
	*/
	void * metacallfs_adaptor(const std::string & name, const std::string & args, void * allocator);

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
		*  @param[in] tag
		*    Loader tag reference
		*
		*  @param[in] scripts
		*    Reference to list of scripts to be iterated
		*/
		parameter_iterator(application & app, const std::string & tag, script_list & scripts);

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

		application & app;		/**< Reference to the application */

		std::string tag;		/**< Loader tag reference */

		script_list & scripts;	/**< Reference to the script list */
	};

	/* -- Private Member Data -- */

	bool exit_condition;	/**< Condition for main loop */

	script_list scripts;	/**< Vector containing a list of script names */

	command_table commands;	/**< Hash table from command strings to command handlers */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_APPLICATION_HPP */
