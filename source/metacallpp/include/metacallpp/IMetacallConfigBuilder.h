/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_IMETACALL_CONFIG_BUILDER_HPP
#define METACALLPP_IMETACALL_CONFIG_BUILDER_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IMetacall.h>

#include <string>

namespace Beast {

	class METACALLPP_API IMetacallConfigBuilder
	{
	public:

		virtual IMetacallConfigBuilder* SetFunctionName(const char *) = 0;

		virtual IMetacallConfigBuilder* SetInvokeType(MetacallInvokeTypes) = 0;
		
		virtual IMetacallConfigBuilder* SetParameterType(const char *) = 0;
		
		virtual IMetacallConfigBuilder* SetReturnType(const char *) = 0;

		virtual ~IMetacallConfigBuilder()
		{
		}
	};
}

#endif /* METACALLPP_IMETACALL_CONFIG_BUILDER_HPP */
