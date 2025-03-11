/*
 *	CLI Command Plugin by Parra Studios
 *	A plugin implementing command line functionality for MetaCall CLI.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef CLI_CMD_PLUGIN_H
#define CLI_CMD_PLUGIN_H 1

#include <metacall/metacall.h>

#define cli_cmd_plugin() metacall_handle("node", "plugins/cli/internal/cli_cmd_plugin/cli_cmd_plugin.js")

#define cli_cmd_plugin_register(cli_cmd_handle, name, type, destroy) \
	do \
	{ \
		const size_t map_size = destroy == NULL ? 1 : 2; \
\
		void *args[] = { \
			metacall_value_create_string(name, sizeof(name) - 1), \
			metacall_value_create_map(NULL, map_size) \
		}; \
\
		void **options = metacall_value_to_map(args[1]); \
		options[0] = metacall_value_create_array(NULL, 2); \
		void **type_tupla = metacall_value_to_array(options[0]); \
\
		static const char type_key[] = "type"; \
		static const char type_value[] = type; \
\
		type_tupla[0] = metacall_value_create_string(type_key, sizeof(type_key) - 1); \
		type_tupla[1] = metacall_value_create_string(type_value, sizeof(type_value) - 1); \
\
		if (destroy != NULL) \
		{ \
			options[1] = metacall_value_create_array(NULL, 2); \
			void **destroy_tupla = metacall_value_to_array(options[1]); \
\
			static const char destroy_key[] = "destroy"; \
\
			destroy_tupla[0] = metacall_value_create_string(destroy_key, sizeof(destroy_key) - 1); \
			destroy_tupla[1] = metacall_value_create_function(metacall_handle_function(handle, destroy)); \
		} \
\
		void *ret = metacallhv_s(cli_cmd_handle, "command_register", args, sizeof(args) / sizeof(args[0])); \
\
		metacall_value_destroy(ret); \
		metacall_value_destroy(args[0]); \
		metacall_value_destroy(args[1]); \
\
	} while (0)

#endif /* CLI_CMD_PLUGIN_H */
