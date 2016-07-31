#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IMetacall.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>
#include <string>

namespace Beast {

	class MetacallConfig
		: public IMetacallConfig,private IMetacallConfigBuilder
	{
	public:

		MetacallConfig();

		MetacallConfig(std::string functionName);

		std::string GetFunctionName();

		MetacallInvokeTypes GetInvokeType();

		std::list<std::string> GetParametersTypes();

		std::string GetReturnType();

		IMetacallConfigBuilder* GetBuilder();

		IMetacallConfigBuilder* SetFunctionName(std::string);

		IMetacallConfigBuilder* SetInvokeType(MetacallInvokeTypes);

		IMetacallConfigBuilder* SetParameterType(std::string);

		IMetacallConfigBuilder* SetReturnType(std::string);

		~MetacallConfig();

	private:

		std::string functionName;

		MetacallInvokeTypes invokeType;

		std::list<std::string> parametersTypes;

		std::string returnType;

	};
}
