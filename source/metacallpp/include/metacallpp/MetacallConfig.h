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

	class MetacallConfig
		: public IMetacallConfig,private IMetacallConfigBuilder
	{
	public:

		METACALLPP_API MetacallConfig();

		/* TODO: Remove STD from class interface in order to be exportable */
		MetacallConfig(std::string functionName);

		METACALLPP_API const char * GetFunctionName();

		METACALLPP_API MetacallInvokeTypes GetInvokeType();

		/* TODO: Remove STD from class interface in order to be exportable */
		std::list<std::string> GetParametersTypes();

		METACALLPP_API const char * GetReturnType();

		METACALLPP_API IMetacallConfigBuilder* GetBuilder();

		METACALLPP_API IMetacallConfigBuilder* SetFunctionName(const char * name);

		METACALLPP_API IMetacallConfigBuilder* SetInvokeType(MetacallInvokeTypes);

		METACALLPP_API IMetacallConfigBuilder* SetParameterType(const char * param);

		METACALLPP_API IMetacallConfigBuilder* SetReturnType(const char * type);

		METACALLPP_API ~MetacallConfig();

	private:

		std::string functionName;

		MetacallInvokeTypes invokeType;

		std::list<std::string> parametersTypes;

		std::string returnType;

	};
}

#endif /* METACALLPP_METACALL_CONFIG_HPP */
