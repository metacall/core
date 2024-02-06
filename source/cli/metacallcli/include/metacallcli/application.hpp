/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_APPLICATION_HPP
#define METACALL_CLI_APPLICATION_HPP 1

/* -- Headers -- */

#include <metacall/metacall.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

/* -- Namespace -- */

namespace metacallcli
{
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
	application(int argc, char *argv[]);

	/**
	*  @brief
	*    Application class destructor
	*/
	~application(void);

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
	*    Create a new value from arguments with parser @p
	*
	*  @param[in] p
	*    Parser which points to the current iterator of the string
	*
	*  @return
	*    Return a new value instanced if argument was correct
	*/
	void *argument_parse(parser_parameter &p);

protected:
	/* -- Protected Methods -- */

	/**
	*  @brief
	*    Load all plugins from a subfolder @path
	*
	*  @param[in] path
	*    Subpath where the plugins are located
	*
	*  @param[out] handle
	*    Pointer to the handle containing of the loaded scripts
	*
	*  @return
	*    Return true if the load was successful, false otherwise
	*
	*/
	bool load_path(const char *path, void **handle);

	/**
	*  @brief
	*    Print a value @v in a readable form on error (throwable)
	*
	*  @param[in] v
	*    Value to be printed
	*
	*/
	void print(void *v);

	/**
	*  @brief
	*    Invoke a function and print the result on error
	*
	*  @param[in] func
	*    Command name matching the function registered from the plugins
	*
	*  @param[in] args
	*    Function arguments as values
	*
	*  @param[in] size
	*    Size of arguments
	*
	*/
	void invoke(const char *func, void *args[], size_t size);

	/**
	*  @brief
	*    Execute a command with string parameters
	*
	*  @param[inout] tokens
	*    Array of values containing all the tokens of the input command
	*
	*  @return
	*    Return result of the command execution
	*/
	void *execute(void *tokens);

private:
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
		*/
		parameter_iterator(application &app);

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
		void operator()(const char *parameter);

		/**
		*  @brief
		*    Assignement operator for parameter iterator
		*
		*  @return
		*    Returns a reference to itself
		*/
		parameter_iterator &operator=(const parameter_iterator &) = delete;

	private:
		/* -- Private Member Data -- */

		application &app; /**< Reference to the application */
	};

	/* -- Private Member Data -- */

	void *plugin_cli_handle;			/**< Handle containing all loaded plugins for CLI */
	void *plugin_repl_handle;			/**< Handle containing all loaded plugins for REPL */
	std::vector<std::string> arguments; /**< Vector containing a list of arguments */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_APPLICATION_HPP */
