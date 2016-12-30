/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <js_loader/js_loader_impl_guard.hpp>

enum js_parser_state
{
	Function, FunctionName, Params, Return, Reset
};

std::string & function_name()
{
	static std::string func("function");

	return func;
}

bool js_loader_impl_guard_parse(std::string & source, std::map<std::string, js_function *> & result, std::string & output)
{
	/* loopzer was here */

	js_parser_state state = js_parser_state::Function;

	size_t function_index = 0;

	bool reading_type = false;

	js_function * current_function;

	parameter_list parameters;

	js_parameter parameter;

	int depth = 0;

	std::string name;

	std::string return_type;

	size_t length = source.size();

	char iterator;

	output.clear();

	for (size_t i = 0; i < length; ++i)
	{
		iterator = source[i];

		if ((iterator != ' ') && (iterator != '\t') && (iterator != '\n') && (iterator != '\r'))
		{
			switch (state)
			{
				case Function :
				{
					std::string & func_name = function_name();

					if (iterator == func_name[function_index])
					{
						++function_index;

						if (function_index == func_name.size())
						{
							function_index = 0;

							state = js_parser_state::FunctionName;

							name.clear();
						}
					}
					else
					{
						function_index = 0;
					}

					break;
				}

				case FunctionName :
				{
					if (iterator == '(')
					{
						if (name.empty())
						{
							state = js_parser_state::Function;

							function_index = 0;
						}
						else
						{
							state = js_parser_state::Params;

							reading_type = false;

							parameter.index = 0;
							parameter.name = std::string();
							parameter.type = std::string();
						}
					}
					else
					{
						name.append(1, iterator);
					}

					break;
				}

				case Params :
				{
					if (iterator == ')')
					{
						if (reading_type)
						{
							parameters.push_back(parameter);
						}

						/*
						if (!parameters.empty())
						{
							for (js_parameter var : params)
							{
								std::cout << var.name << std::endl;
								std::cout << var.type << std::endl;
								std::cout << var.index << std::endl;
							}
						}
						*/

						state = js_parser_state::Return;

						return_type.clear();

						reading_type = false;

						/*
						std::cout << "state:" << state << std::endl;
						*/
					}
					else
					{
						if (iterator == ',')
						{
							parameters.push_back(parameter);

							++parameter.index;
							parameter.name = std::string();
							parameter.type = std::string();

							reading_type = false;
						}
						else
						{
							if (iterator == ':')
							{
								reading_type = true;
							}
							else
							{
								if (reading_type)
								{
									parameter.type.append(1, iterator);
								}
								else
								{
									parameter.name.append(1, iterator);
								}
							}
						}
					}

					break;
				}

				case Return :
				{
					if (iterator == '{')
					{
						reading_type = false;

						state = js_parser_state::Function;

						current_function = new js_function;

						current_function->name = name;

						current_function->return_type = return_type;

						for (js_parameter var : parameters)
						{
							current_function->parameters.push_back(var);
						}

						result.insert(
							std::pair<std::string, js_function *>(
								current_function->name,
								current_function)
						);

						parameters = parameter_list();

						state = js_parser_state::Reset;

						depth = 1;
					}
					else
					{
						reading_type = true;

						if (iterator != ':')
						{
							return_type.append(1, iterator);
						}
					}

					break;
				}

				case Reset :
				{
					if (iterator == '{')
					{
						++depth;
					}

					if (iterator == '}')
					{
						--depth;
					}

					if (depth == 0)
					{
						state = js_parser_state::Function;
					}

					break;
				}

				default :
				{
					return false;
				}
			}
		}

		if (reading_type == false)
		{
			output.append(1, iterator);
		}
	}

	return true;
}
