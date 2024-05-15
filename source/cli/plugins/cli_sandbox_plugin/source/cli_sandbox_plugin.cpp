/*
 *	CLI Sandbox Plugin by Parra Studios
 *	A plugin implementing sandboxing functionality for MetaCall CLI.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <cli_sandbox_plugin/cli_sandbox_plugin.h>

#include <cli_cmd_plugin/cli_cmd_plugin.hpp>

#include <plugin/plugin_interface.hpp>

#define SANDBOXING_NOT_INITIALIZED_ERROR "Sandboxing has not been initialized, enable it by passing --sandboxing to the CLI"

void *sandboxing_handle = NULL;
void *sandboxing_context = NULL;

void *sandboxing(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to initialize sandboxing", METACALL_BOOL);

	void *init_args[] = {
		metacall_value_create_bool(1L) /* Allow everything by default */
	};

	sandboxing_context = metacallhv_s(sandboxing_handle, "sandbox_initialize", init_args, sizeof(init_args) / sizeof(init_args[0]));

	metacall_value_destroy(init_args[0]);

	if (metacall_value_id(sandboxing_context) != METACALL_PTR)
	{
		/* It is an exception, rethrow it */
		return sandboxing_context;
	}

	return metacall_value_create_int(0);
}

void *sandboxing_destroy(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to destroy sandboxing");

	if (sandboxing_context != NULL)
	{
		void *destroy_args[] = {
			sandboxing_context
		};

		/* Destroy sandboxing context */
		void *ret = metacallhv_s(sandboxing_handle, "sandbox_destroy", destroy_args, sizeof(destroy_args) / sizeof(destroy_args[0]));

		metacall_value_destroy(sandboxing_context);

		/* Clear sandboxing data for future use (in case of reinitialization) */
		sandboxing_context = NULL;
		sandboxing_handle = NULL;

		return ret;
	}

	return metacall_value_create_int(0);
}

void *disable_filesystem(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to disable filesystem", METACALL_BOOL);

	/* If context is not initialized, skip */
	if (sandboxing_context == NULL)
	{
		EXTENSION_FUNCTION_THROW(SANDBOXING_NOT_INITIALIZED_ERROR);
	}

	/* Disable filesystem */
	void *fs_args[] = {
		sandboxing_context,
		metacall_value_create_bool(0L)
	};

	void *ret = metacallhv_s(sandboxing_handle, "sandbox_filesystems", fs_args, sizeof(fs_args) / sizeof(fs_args[0]));

	metacall_value_destroy(fs_args[1]);

	return ret;
}

void *disable_io(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to disable io", METACALL_BOOL);

	/* If context is not initialized, skip */
	if (sandboxing_context == NULL)
	{
		EXTENSION_FUNCTION_THROW(SANDBOXING_NOT_INITIALIZED_ERROR);
	}

	/* Disable io */
	void *io_args[] = {
		sandboxing_context,
		metacall_value_create_bool(0L)
	};

	void *ret = metacallhv_s(sandboxing_handle, "sandbox_io", io_args, sizeof(io_args) / sizeof(io_args[0]));

	metacall_value_destroy(io_args[1]);

	return ret;
}

void *disable_time(size_t argc, void *args[], void *data)
{
	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK("Failed to disable time", METACALL_BOOL);

	/* If context is not initialized, skip */
	if (sandboxing_context == NULL)
	{
		EXTENSION_FUNCTION_THROW(SANDBOXING_NOT_INITIALIZED_ERROR);
	}

	/* Disable time */
	void *time_args[] = {
		sandboxing_context,
		metacall_value_create_bool(0L)
	};

	void *ret = metacallhv_s(sandboxing_handle, "sandbox_time", time_args, sizeof(time_args) / sizeof(time_args[0]));

	metacall_value_destroy(time_args[1]);

	return ret;
}

int cli_sandbox_plugin(void *loader, void *handle)
{
	/* Initialize the sandbox plugin */
	sandboxing_handle = metacall_handle("ext", "plugins/sandbox_plugin/sandbox_plugin");

	if (sandboxing_handle != NULL)
	{
		/* Get CMD handle */
		void *cli_cmd_handle = cli_cmd_plugin();

		if (cli_cmd_handle == NULL)
		{
			return 1;
		}

		/* Register functions */
		EXTENSION_FUNCTION(METACALL_INT, sandboxing, METACALL_BOOL);
		EXTENSION_FUNCTION(METACALL_INT, disable_filesystem, METACALL_BOOL);
		EXTENSION_FUNCTION(METACALL_INT, disable_io, METACALL_BOOL);
		EXTENSION_FUNCTION(METACALL_INT, disable_time, METACALL_BOOL);
		EXTENSION_FUNCTION(METACALL_INT, sandboxing_destroy);

		/* Register sandboxing command */
		cli_cmd_plugin_register(cli_cmd_handle, "sandboxing", "METACALL_BOOL", "sandboxing_destroy");

		/* Register disable_filesystem command */
		cli_cmd_plugin_register(cli_cmd_handle, "disable_filesystem", "METACALL_BOOL", NULL);

		/* Register disable_io command */
		cli_cmd_plugin_register(cli_cmd_handle, "disable_io", "METACALL_BOOL", NULL);

		/* Register disable_time command */
		cli_cmd_plugin_register(cli_cmd_handle, "disable_time", "METACALL_BOOL", NULL);
	}

	return 0;
}
