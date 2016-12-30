/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_IMETACALL_CONFIG_HPP
#define METACALLPP_IMETACALL_CONFIG_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>
#include <string>

namespace Beast {

	class METACALLPP_API IMetacallConfig
	{
	public:

		virtual const char * GetFunctionName() = 0;

		virtual MetacallInvokeTypes GetInvokeType() = 0;

		virtual std::list<std::string> GetParametersTypes() = 0;

		virtual const char * GetReturnType() = 0;

		virtual IMetacallConfigBuilder* GetBuilder() = 0;

		virtual ~IMetacallConfig()
		{
		}
	};
}

#endif /* METACALLPP_IMETACALL_CONFIG_HPP */
