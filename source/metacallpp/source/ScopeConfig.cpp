#include <metacallpp/ScopeConfig.h>

namespace Beast {

	ScopeConfig::ScopeConfig(MetacallInvokeTypes type)
	{
		this->metacallInvokeType = type;
	}

	ScopeConfig::~ScopeConfig() {

	}

	MetacallInvokeTypes ScopeConfig::GetMetacallType() {
		return this->metacallInvokeType;
	}

	IMetacallConfigBuilder * ScopeConfig::CreateMetacallConfig() {
		MetacallConfig * m = new MetacallConfig();
		this->metacallConfigs.push_back(m);
		return m->GetBuilder();
	}

	std::list<IMetacallConfig *>* ScopeConfig::GetMetacallConfigs() {
		return &this->metacallConfigs;
	}
}
