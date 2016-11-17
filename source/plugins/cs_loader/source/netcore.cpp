#include <cs_loader/netcore.h>
#include <stdlib.h>
#include <ostream>
#include <iostream>
netcore::netcore()
{
}


netcore::~netcore()
{
}

reflect_function * netcore::get_functions(int * count) {

	this->core_get_functions(&this->functions_count, this->functions);

	for (size_t i = 0; i < this->functions_count; i++)
	{
		std::cout << "function name2:" << this->functions[i].name << std::endl;
		std::cout << "function return type:" << this->functions[i].return_type << std::endl;
		std::cout << "function param count:" << this->functions[i].param_count << std::endl;

		for (size_t p = 0; p < this->functions[i].param_count; p++)
		{
			std::cout << "params:" << this->functions[i].pars[p].name << std::endl;
		}

	}

	*count = this->functions_count;

	return this->functions;
}

bool netcore::create_delegates() {
	if (!this->create_delegate(this->delegate_get_functions, (void**)&this->core_get_functions)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_load_w, (void**)&this->core_load_w)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_load_c, (void**)&this->core_load_c)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_execute_w, (void**)&this->execute_w)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_execute_c, (void**)&this->execute_c)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_execute_with_params_w, (void**)&this->execute_with_params_w)) {
		return false;
	}
	if (!this->create_delegate(this->delegate_execute_with_params_c, (void**)&this->execute_with_params_c)) {
		return false;
	}

	return true;
}


bool netcore::load_source(wchar_t * source) {
	return this->core_load_w(source) ? true : false;
};
bool netcore::load_source(char * source) {
	return this->core_load_c(source) ? true : false;
}

bool netcore::execute(char * function) {
	std::cout << "invoke name:" << function << std::endl;
	return this->execute_c(function) ? true : false;
}
bool netcore::execute(wchar_t * function) {
	return this->execute_w(function) ? true : false;
}
bool netcore::execute_with_params(char * function, parameters * params, size_t size) {
	return this->execute_with_params_c(function, params, (short)size) ? true : false;
}
bool netcore::execute_with_params(wchar_t * function, parameters * params, size_t size) {
	return this->execute_with_params_w(function, params, (short)size) ? true : false;
}