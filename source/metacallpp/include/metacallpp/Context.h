/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_CONTEXT_HPP
#define METACALLPP_CONTEXT_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IContext.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/ScopeBase.h>

#include <iostream>
#include <string>

namespace Beast {

	class METACALLPP_API Context :
		public IContext
	{
	public:
		Context();
		~Context();

		ScopeBase * CreateScope(IScopeConfig* config);
	};

}

#endif /* METACALLPP_CONTEXT_HPP */
