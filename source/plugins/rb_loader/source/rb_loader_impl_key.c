/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#include <rb_loader/rb_loader_impl_key.h>

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

/* -- Private Methods -- */

static const char * rb_loader_impl_function_begin(void);

static const char * rb_loader_impl_function_do(void);

static const char * rb_loader_impl_function_end(void);

/* -- Methods -- */

const char * rb_loader_impl_function_begin()
{
	static const char func_begin_name[] = "def";

	return func_begin_name;
}

const char * rb_loader_impl_function_do()
{
	static const char func_do_name[] = "do";

	return func_do_name;
}

const char * rb_loader_impl_function_end()
{
	static const char func_end_name[] = "end";

	return func_end_name;
}

int rb_loader_impl_key_parse(const char * source, set function_map)
{
	enum rb_loader_impl_parser_state state = rb_loader_impl_parser_state_function;

	size_t function_keyword_index = 0, function_index = 0;

	char function_name[RB_LOADER_IMPL_PARSER_FUNC];

	size_t iterator, length = strlen(source);

	struct rb_function_parameter_parser_type parameters[RB_LOADER_IMPL_PARSER_PARAM];

	struct rb_function_parameter_parser_type parameter;

	size_t parameter_size;

	int reading_parameter_type = 1;

	size_t reset_depth = 1;

	size_t parameter_type_size = 0, parameter_name_size = 0;

	for (iterator = 0; iterator < length; ++iterator)
	{
		const char character = source[iterator];

		if ((character != ' ') && (character != '\t') &&
			(character != '\n' || (character == '\n' && state == rb_loader_impl_parser_state_function_name)) &&
			(character != '\r'))
		{
			switch (state)
			{
				case rb_loader_impl_parser_state_function :
				{
					const char * function_keyword = rb_loader_impl_function_begin();

					if (character == function_keyword[function_keyword_index])
					{
						++function_keyword_index;

						state = rb_loader_impl_parser_state_function_name;

						memset(function_name, 0, RB_LOADER_IMPL_PARSER_FUNC);
					}
					else
					{
						function_keyword_index = 0;
					}

					break;
				}

				case rb_loader_impl_parser_state_function_name :
				{
					if (character == '(')
					{
						if (function_index == 0)
						{
							state = rb_loader_impl_parser_state_function;

							function_keyword_index = 0;
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

							function_keyword_index = 0;
						}
						else
						{
							state = rb_loader_impl_parser_state_reset;

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

				case rb_loader_impl_parser_state_params :
				{
					if (character == ')')
					{
						rb_function_parser function;

						state = rb_loader_impl_parser_state_reset;

						if (reading_parameter_type == 0)
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

						if (set_insert(function_map, function->name, function) != 0)
						{
							log_write("metacall", LOG_LEVEL_ERROR, "Invalid ruby parser function map insertion");
						}

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

				case rb_loader_impl_parser_state_reset :
				{
					const char * func_do = rb_loader_impl_function_do();

					const char * func_def = rb_loader_impl_function_begin();

					const char * func_end = rb_loader_impl_function_end();

					if (strcmp(&source[iterator], func_do) == 0 ||
						strcmp(&source[iterator], func_def) == 0)
					{
						++reset_depth;
					}

					if (strcmp(&source[iterator], func_end) == 0)
					{
						--reset_depth;
					}

					if (reset_depth == 0)
					{
						state = rb_loader_impl_parser_state_function;
					}

					break;
				}

				default :
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int rb_loader_impl_key_print(set function_map)
{
	if (set_size(function_map) > 0)
	{
		set_iterator iterator = set_iterator_begin(function_map);

		do
		{
			size_t parameter;

			rb_function_parser function = set_iterator_get_value(iterator);

			log_write("metacall", LOG_LEVEL_DEBUG, "Ruby loader key parse function (%s)", function->name);

			for (parameter = 0; parameter < function->params_size; ++parameter)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "	Ruby loader key parse parameter [%d] (%s : %s)",
					function->params[parameter].index,
					function->params[parameter].name,
					function->params[parameter].type);
			}

			set_iterator_next(iterator);

		} while (set_iterator_end(&iterator) != 0);
	}

	return 0;
}

int rb_loader_impl_key_clear(set function_map)
{
	if (set_size(function_map) > 0)
	{
		set_iterator iterator = set_iterator_begin(function_map);

		do
		{
			rb_function_parser function = set_iterator_get_value(iterator);

			if (function != NULL)
			{
				free(function);
			}

			set_iterator_next(iterator);

		} while (set_iterator_end(&iterator) != 0);

		return set_clear(function_map);
	}

	return 0;
}
