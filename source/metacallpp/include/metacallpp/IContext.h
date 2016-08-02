/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_ICONTEXT_HPP
#define METACALLPP_ICONTEXT_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/ScopeBase.h>
#include <metacallpp/IScopeConfig.h>

#include <iostream>

namespace Beast {

	class METACALLPP_API IContext
	{
	public:

		virtual ScopeBase * CreateScope(IScopeConfig * config) = 0;

		virtual ~IContext()
		{

		}
	};
}

#endif /* METACALLPP_ICONTEXT_HPP */
