#include <cs_loader/netcore.h>
#include <iostream>
#include <exception>

netcore::netcore(char * dotnet_root,char * dotnet_loader_assembly_path)
{
	this->dotnet_root = dotnet_root;
	this->dotnet_loader_assembly_path = dotnet_loader_assembly_path;
}

netcore::~netcore()
{

}

reflect_function * netcore::get_functions(int * count)
{
	this->core_get_functions(&this->functions_count, this->functions);

	for (size_t i = 0; i < this->functions_count; ++i)
	{
		std::cout << "function name:" << this->functions[i].name << std::endl;
		std::cout << "function return type:" << this->functions[i].return_type << std::endl;
		std::cout << "function param count:" << this->functions[i].param_count << std::endl;

		for (size_t p = 0; p < this->functions[i].param_count; ++p)
		{
			std::cout << "params:" << this->functions[i].pars[p].name << std::endl;
		}
	}

	*count = this->functions_count;

	return this->functions;
}

bool netcore::create_delegates()
{
	if (!this->create_delegate(this->delegate_get_functions, (void**)&this->core_get_functions))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_source_w, (void**)&this->core_load_from_source_w))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_source_c, (void**)&this->core_load_from_source_c))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_files_w, (void**)&this->core_load_from_files_w))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_files_c, (void**)&this->core_load_from_files_c))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_assembly_w, (void**)&this->core_load_from_assembly_w))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_load_assembly_c, (void**)&this->core_load_from_assembly_c))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_w, (void**)&this->execute_w))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_c, (void**)&this->execute_c))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_with_params_w, (void**)&this->execute_with_params_w))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_execute_with_params_c, (void**)&this->execute_with_params_c))
	{
		return false;
	}

	if (!this->create_delegate(this->delegate_destroy_execution_result, (void**)&this->core_destroy_execution_result))
	{
		return false;
	}

	return true;
}

bool netcore::load_source(wchar_t * source)
{
	try
	{
		return this->core_load_from_source_w(source) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

bool netcore::load_source(char * source)
{
	try
	{
		return this->core_load_from_source_c(source) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

bool netcore::load_files(wchar_t ** source, size_t size)
{
	try
	{
		return this->core_load_from_files_w(source, size) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

bool netcore::load_files(char ** source, size_t size)
{
	try
	{
		return this->core_load_from_files_c(source, size) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

bool netcore::load_assembly(wchar_t * source)
{
	try
	{
		return this->core_load_from_assembly_w(source) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

bool netcore::load_assembly(char * source)
{
	try
	{
		return this->core_load_from_assembly_c(source) > 0 ? true : false;
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return false;
}

execution_result * netcore::execute(char * function)
{
	try
	{
		return this->execute_c(function);
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return NULL;
}
execution_result * netcore::execute(wchar_t * function)
{
	try
	{
		return this->execute_w(function);
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return NULL;
}
execution_result * netcore::execute_with_params(char * function, parameters * params)
{
	try
	{
		return  this->execute_with_params_c(function, params);
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return NULL;
}

execution_result * netcore::execute_with_params(wchar_t * function, parameters * params)
{
	try
	{
		return this->execute_with_params_w(function, params);
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}

	return NULL;
}

void netcore::destroy_execution_result(execution_result * er)
{
	try
	{
		this->core_destroy_execution_result(er);
	}
	catch (const std::exception & ex)
	{
		std::cerr << ex.what();
	}
}
