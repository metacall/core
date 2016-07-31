#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>

namespace Beast {
	class IScopeConfig
	{
	public:

		virtual MetacallInvokeTypes GetMetacallType() = 0;

		virtual IMetacallConfigBuilder * CreateMetacallConfig() = 0;

		virtual std::list<IMetacallConfig *>* GetMetacallConfigs() = 0;

		virtual ~IScopeConfig()
		{
		}
	};
}
