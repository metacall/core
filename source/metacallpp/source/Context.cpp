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