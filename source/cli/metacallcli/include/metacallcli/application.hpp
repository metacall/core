/*
 *	MetaCall Command Line Interface by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A command line interface example as metacall wrapper.
 *
 */

#ifndef METACALL_CLI_APPLICATION_HPP
#define METACALL_CLI_APPLICATION_HPP 1

/* -- Headers -- */

#include <metacall/metacall.h>

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
	static std::unordered_map<std::string, void *> function_map;

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
	*    Application script loader
	*
	*  @param[in] script
	*    Reference to script name
	*
	*  @return
	*    Return true on success, false otherwhise
	*/
	bool load(const std::string script);

	/**
	*  @brief
	*    Application main entry point
	*/
	void run(void);

	/**
	*  @brief
	*    Shutdown main application loop
	*/
	void shutdown(int exitcode = 0, std::string message = "");

private: /* -- Private Type Definitions -- */
	typedef std::vector<std::string> arg_list;

	typedef std::vector<std::string> script_list;

	/* -- Private Member Data -- */

	bool exit_condition;	 /**< Condition for main loop */
	arg_list arguments;		 /**< Vector containing a list of arguments */
	script_list scripts;	 /**< Vector containing a list of script names */
	void *plugin_cli_handle; /**< Handle containing all loaded plugins */
};

} /* namespace metacallcli */

#endif /* METACALL_CLI_APPLICATION_HPP */
