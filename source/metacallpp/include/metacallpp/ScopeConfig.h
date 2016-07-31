#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>
#include <metacallpp/MetacallConfig.h>

#include <list>

namespace Beast {
	class ScopeConfig :
		public IScopeConfig

	{
	public:

		ScopeConfig(MetacallInvokeTypes type);

		MetacallInvokeTypes GetMetacallType();

		IMetacallConfigBuilder * CreateMetacallConfig();

		std::list<IMetacallConfig *>* GetMetacallConfigs();

		~ScopeConfig();

	private:
		MetacallInvokeTypes metacallInvokeType;

		std::list<IMetacallConfig *> metacallConfigs;
	};
}
