/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

#include <rb_loader/rb_loader_impl_parser.h>

#include <log/log.h>

#include <string.h>

/* -- Enumerations -- */

enum rb_loader_impl_parser_state
{
	rb_loader_impl_parser_state_function,
	rb_loader_impl_parser_state_function_name,
	rb_loader_impl_parser_state_params,
	rb_loader_impl_parser_state_reset
};

enum rb_loader_impl_comment_state
{
	rb_loader_impl_comment_state_none,
	rb_loader_impl_comment_state_line,
	rb_loader_impl_comment_state_multi_line,
	rb_loader_impl_comment_state_multi_line_end
};

/* -- Methods -- */

int rb_loader_impl_key_parse(const char *source, set function_map)
{
	static const char func_def_name[] = "def", func_do_name[] = "do", func_end_name[] = "end";
	static const char comment_begin[] = "begin\n", comment_end[] = "end\n";

	enum rb_loader_impl_parser_state state = rb_loader_impl_parser_state_function;
	enum rb_loader_impl_comment_state comment = rb_loader_impl_comment_state_none;

	size_t func_def_index = 0, func_do_index = 0, func_end_index = 0;
	size_t function_index = 0;

	char function_name[RB_LOADER_IMPL_PARSER_FUNC];

	size_t iterator, length = strlen(source);

	struct rb_function_parameter_parser_type parameters[RB_LOADER_IMPL_PARSER_PARAM];

	struct rb_function_parameter_parser_type parameter;

	size_t parameter_size = 0;

	int reading_parameter_type = 1;

	size_t reset_depth = 1;

	size_t parameter_type_size = 0, parameter_name_size = 0;

	size_t comment_begin_index = 0, comment_end_index = 0;

	int comment_multi_line = 1;

	for (iterator = 0; iterator < length; ++iterator)
	{
		const char character = source[iterator];

		switch (comment)
		{
			case rb_loader_impl_comment_state_none: {
				if (character == '#')
				{
					comment = rb_loader_impl_comment_state_line;
				}
				else if (character == '=' && (iterator == 0 || (iterator > 0 && source[iterator - 1] == '\n')))
				{
					if (comment_multi_line == 1)
					{
						comment = rb_loader_impl_comment_state_multi_line;

						comment_begin_index = 0;
					}
					else
					{
						comment = rb_loader_impl_comment_state_multi_line_end;

						comment_end_index = 0;
					}
				}

				break;
			}

			case rb_loader_impl_comment_state_line: {
				if (character == '\n')
				{
					comment = rb_loader_impl_comment_state_none;
				}

				break;
			}

			case rb_loader_impl_comment_state_multi_line: {
				if (character == comment_begin[comment_begin_index])
				{
					++comment_begin_index;

					if (comment_begin_index == sizeof(comment_begin) - 1)
					{
						comment = rb_loader_impl_comment_state_none;

						comment_multi_line = 0;

						comment_end_index = 0;
					}
				}
				else
				{
					comment_begin_index = 0;

					comment = rb_loader_impl_comment_state_none;
				}

				break;
			}

			case rb_loader_impl_comment_state_multi_line_end: {
				if (character == comment_end[comment_end_index])
				{
					++comment_end_index;

					if (comment_end_index == sizeof(comment_end) - 1)
					{
						comment = rb_loader_impl_comment_state_none;

						comment_multi_line = 1;

						comment_end_index = 0;
					}
				}
				else
				{
					comment_end_index = 0;

					comment = rb_loader_impl_comment_state_none;
				}

				break;
			}
		}

		if ((comment == rb_loader_impl_comment_state_none && comment_multi_line == 1) &&
			((character != ' ') && (character != '\t') &&
				(character != '\n' || (character == '\n' && state == rb_loader_impl_parser_state_function_name)) &&
				(character != '\r')))
		{
			switch (state)
			{
				case rb_loader_impl_parser_state_function: {
					if (character == func_def_name[func_def_index])
					{
						++func_def_index;

						if (func_def_index == sizeof(func_def_name) - 1)
						{
							state = rb_loader_impl_parser_state_function_name;

							memset(function_name, 0, RB_LOADER_IMPL_PARSER_FUNC);
						}
					}
					else
					{
						func_def_index = 0;
					}

					break;
				}

				case rb_loader_impl_parser_state_function_name: {
					if (character == '(')
					{
						if (function_index == 0)
						{
							state = rb_loader_impl_parser_state_function;

							func_def_index = 0;
						}
						else
						{
							state = rb_loader_impl_parser_state_params;

							parameter_size = function_index = 0;

							reading_parameter_type = 1;

							memset(parameters, 0, RB_LOADER_IMPL_PARSER_PARAM * sizeof(struct rb_function_parameter_parser_type));
							memset(&parameter, 0, sizeof(struct rb_function_parameter_parser_type));
						}
					}
					else if (character == '\n')
					{
						if (function_index == 0)
						{
							state = rb_loader_impl_parser_state_function;

							func_def_index = 0;
						}
						else
						{
							state = rb_loader_impl_parser_state_reset;

							func_def_index = func_do_index = func_end_index = 0;

							reset_depth = 1;

							parameter_size = function_index = 0;
						}
					}
					else
					{
						function_name[function_index] = character;

						++function_index;
					}

					break;
				}

				case rb_loader_impl_parser_state_params: {
					if (character == ')')
					{
						rb_function_parser function;

						state = rb_loader_impl_parser_state_reset;

						if (parameter_name_size > 0)
						{
							parameters[parameter_size].index = parameter.index;
							strncpy(parameters[parameter_size].name, parameter.name, RB_LOADER_IMPL_PARSER_KEY);
							strncpy(parameters[parameter_size].type, parameter.type, RB_LOADER_IMPL_PARSER_TYPE);

							++parameter_size;
						}

						reading_parameter_type = 1;

						parameter_type_size = parameter_name_size = 0;

						function = malloc(sizeof(struct rb_function_parser_type));

						if (function == NULL)
						{
							log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby parser function allocation");
						}

						strncpy(function->name, function_name, RB_LOADER_IMPL_PARSER_FUNC);
						memcpy(function->params, parameters, parameter_size * sizeof(struct rb_function_parameter_parser_type));
						function->params_size = parameter_size;

						/* TODO: This is not skipping class functions, that is a wrong behavior */
						log_write("metacall", LOG_LEVEL_DEBUG, "Inserting parsed Ruby function '%s' into function map", function->name);

						if (set_insert(function_map, function->name, function) != 0)
						{
							log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby parser function map insertion");
						}

						func_def_index = func_do_index = func_end_index = 0;

						reset_depth = 1;
					}
					else
					{
						if (character == ',')
						{
							parameters[parameter_size].index = parameter.index;
							strncpy(parameters[parameter_size].name, parameter.name, RB_LOADER_IMPL_PARSER_KEY);
							strncpy(parameters[parameter_size].type, parameter.type, RB_LOADER_IMPL_PARSER_TYPE);

							++parameter_size;

							++parameter.index;
							memset(parameter.name, 0, RB_LOADER_IMPL_PARSER_KEY);
							memset(parameter.type, 0, RB_LOADER_IMPL_PARSER_TYPE);

							reading_parameter_type = 1;

							parameter_type_size = parameter_name_size = 0;
						}
						else
						{
							if (character == ':')
							{
								reading_parameter_type = 0;
							}
							else
							{
								if (reading_parameter_type == 0)
								{
									parameter.type[parameter_type_size] = character;

									++parameter_type_size;
								}
								else
								{
									parameter.name[parameter_name_size] = character;

									++parameter_name_size;
								}
							}
						}
					}

					break;
				}

				case rb_loader_impl_parser_state_reset: {
					if (character == func_do_name[func_do_index])
					{
						++func_do_index;
					}
					else
					{
						func_do_index = 0;
					}

					if (character == func_def_name[func_def_index])
					{
						++func_def_index;
					}
					else
					{
						func_def_index = 0;
					}

					if (character == func_end_name[func_end_index])
					{
						++func_end_index;
					}
					else
					{
						func_end_index = 0;
					}

					if (func_do_index == sizeof(func_do_name) - 1 || func_def_index == sizeof(func_def_name) - 1)
					{
						++reset_depth;
					}

					if (func_end_index == sizeof(func_end_name) - 1)
					{
						--reset_depth;
					}

					if (reset_depth == 0)
					{
						state = rb_loader_impl_parser_state_function;

						func_def_index = func_do_index = func_end_index = 0;
					}

					break;
				}

				default: {
					return 1;
				}
			}
		}
		else
		{
			func_def_index = func_do_index = func_end_index = 0;
		}
	}

	return 0;
}

void rb_loader_impl_key_print(set function_map)
{
	set_iterator it;

	for (it = set_iterator_begin(function_map); set_iterator_end(&it) != 0; set_iterator_next(it))
	{
		size_t parameter;

		rb_function_parser function = set_iterator_value(it);

		log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader key parse function (%s)", function->name);

		for (parameter = 0; parameter < function->params_size; ++parameter)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "	Ruby loader key parse parameter [%d] (%s : %s)",
				function->params[parameter].index,
				function->params[parameter].name,
				function->params[parameter].type);
		}
	}
}

void rb_loader_impl_key_clear(set function_map)
{
	set_iterator it;

	for (it = set_iterator_begin(function_map); set_iterator_end(&it) != 0; set_iterator_next(it))
	{
		rb_function_parser function = set_iterator_value(it);

		free(function);
	}

	set_destroy(function_map);
}
