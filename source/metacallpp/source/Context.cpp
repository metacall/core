/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

/* -- Headers -- */

#include <metacallpp/Context.h>

namespace Beast {
	Context::Context()
	{
	}


	Context::~Context()
	{
	}

	ScopeBase * Context::CreateScope(IScopeConfig* config) {
		return new ScopeBase(config);
	}
}