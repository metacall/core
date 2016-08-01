/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_METACALL_CONFIG_HPP
#define METACALLPP_METACALL_CONFIG_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IMetacall.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>
#include <string>

namespace Beast {

	class METACALLPP_API MetacallConfig
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

#endif /* METACALLPP_METACALL_CONFIG_HPP */
