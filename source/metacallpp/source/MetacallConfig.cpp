
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