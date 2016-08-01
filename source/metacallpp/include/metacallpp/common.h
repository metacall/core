/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_COMMON_HPP
#define METACALLPP_COMMON_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

namespace Beast {
	enum MetacallStates
	{
		UnInitialize,
		Ready,
		Error
	};

	enum MetacallInvokeTypes
	{
		Fixed,
		Dynamic
	};
}

#endif /* METACALLPP_COMMON_HPP */
