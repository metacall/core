#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IContext.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/ScopeBase.h>

#include <iostream>
#include <string>

namespace Beast {

	class Context :
		public IContext
	{
	public:
		Context();
		~Context();

		ScopeBase * CreateScope(IScopeConfig* config);
	};

}