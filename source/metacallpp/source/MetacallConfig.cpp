/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

/* -- Headers -- */

#include <metacallpp/MetacallConfig.h>

namespace Beast {

	MetacallConfig::MetacallConfig(std::string functionName)
	{
		this->functionName = functionName;
	}

	MetacallConfig::MetacallConfig()
	{
	}

	MetacallConfig::~MetacallConfig()
	{
	}

	const char * MetacallConfig::GetFunctionName() {
		return this->functionName.c_str();
	}

	MetacallInvokeTypes MetacallConfig::GetInvokeType() {
		return this->invokeType;
	}

	std::list<std::string> MetacallConfig::GetParametersTypes() {
		return this->parametersTypes;
	}

	const char * MetacallConfig::GetReturnType() {
		return this->returnType.c_str();
	}

	IMetacallConfigBuilder* MetacallConfig::GetBuilder() {
		return (IMetacallConfigBuilder *)this;
	}

	IMetacallConfigBuilder* MetacallConfig::SetFunctionName(const char * name) {
		this->functionName = std::string(name);
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetInvokeType(MetacallInvokeTypes type) {
		this->invokeType = type;
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetParameterType(const char * param) {
		this->parametersTypes.push_back(std::string(param));
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetReturnType(const char * type) {
		this->returnType = std::string(type);
		return this->GetBuilder();
	}
}