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

protected:
	/* -- Protected Methods -- */

	/**
	*  @brief
	*    Initialize the REPL
	*/
	void repl();

	/**
	*  @brief
	*    Initialize the CMD
	*
	*  @param[in] arguments
	*    Vector of strings containing all the arguments from argv
	*
	*  @return
	*    Return true if the load was successful, false otherwise
	*/
	bool cmd(std::vector<std::string> &arguments);

	/**
	*  @brief
	*    Fallback argument parser
	*
	*  @param[in] arguments
	*    Vector of strings containing all the arguments from argv
	*
	*/
	void arguments_parse_fallback(std::vector<std::string> &arguments);

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
	*    Execute a command with string parameters
	*
	*  @param[inout] tokens
	*    Value of type array containing all the tokens of the input command
	*
	*  @return
	*    Return result of the command execution
	*/
	void *execute(void *tokens);

	/**
	*  @brief
	*    Check if a value is an exception or throwable, then prints it.
	*    The method always destroys the value @v
	*
	*  @param[inout] v
	*    Value to be checked against and destroyed
	*/
	void check_for_exception(void *v);

private:
	/* -- Private Member Data -- */

	void *plugin_cli_handle;  /**< Handle containing all loaded plugins for CLI */
	void *plugin_repl_handle; /**< Handle containing all loaded plugins for REPL */
	void *plugin_cmd_handle;  /**< Handle containing all loaded plugins for CMD */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_APPLICATION_HPP */
