/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_IMETACALL_HPP
#define METACALLPP_IMETACALL_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/ICall.h>

#include <string>

namespace Beast {

	class METACALLPP_API IMetacall
	{
	public:

		virtual ICall * Prepare() = 0;

		virtual const char * GetFunctionName() = 0;

		virtual MetacallStates GetState() = 0;

		virtual bool IsReady() = 0;

		virtual ~IMetacall()
		{
		}
	};
}

#endif /* METACALLPP_IMETACALL_HPP */
