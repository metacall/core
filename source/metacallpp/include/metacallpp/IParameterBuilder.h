/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_IPARAMETER_BUILDER_HPP
#define METACALLPP_IPARAMETER_BUILDER_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>

#include <iostream>

namespace Beast {

	class METACALLPP_API IParameterBuilder
	{
	public:
		virtual IParameterBuilder * Param(int value) = 0;
		virtual IParameterBuilder * Param(long value) = 0;
		virtual IParameterBuilder * Param(const char * value) = 0;

		virtual ~IParameterBuilder()
		{
		}
	};
}

#endif /* METACALLPP_IPARAMETER_BUILDER_HPP */
