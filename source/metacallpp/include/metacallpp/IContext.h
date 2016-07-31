#pragma once

#include <metacallpp/ScopeBase.h>
#include <metacallpp/IScopeConfig.h>

#include <iostream>

namespace Beast {

	class IContext
	{
	public:

		virtual ScopeBase * CreateScope(IScopeConfig * config) = 0;

		~IContext()
		{
		}
	};
}