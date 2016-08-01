/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

/* -- Headers -- */

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
