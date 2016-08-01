/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

	std::string MetacallConfig::GetFunctionName() {
		return this->functionName;
	}

	MetacallInvokeTypes MetacallConfig::GetInvokeType() {
		return this->invokeType;
	}

	std::list<std::string> MetacallConfig::GetParametersTypes() {
		return this->parametersTypes;
	}

	std::string MetacallConfig::GetReturnType() {
		return this->returnType;
	}

	IMetacallConfigBuilder* MetacallConfig::GetBuilder() {
		return (IMetacallConfigBuilder *)this;
	}

	IMetacallConfigBuilder* MetacallConfig::SetFunctionName(std::string name) {
		this->functionName = name;
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetInvokeType(MetacallInvokeTypes type) {
		this->invokeType = type;
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetParameterType(std::string param) {
		this->parametersTypes.push_back(param);
		return this->GetBuilder();
	}

	IMetacallConfigBuilder* MetacallConfig::SetReturnType(std::string type) {
		this->returnType = type;
		return this->GetBuilder();
	}
}