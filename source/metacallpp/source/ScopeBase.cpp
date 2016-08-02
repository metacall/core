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

#include <metacallpp/ScopeBase.h>

namespace Beast {
	using namespace std;

	ScopeBase::ScopeBase(IScopeConfig * config)
	{
		this->config = config;

		/* TODO: solve segmentation fault here */
		for (IMetacallConfig* metaConfig : *this->config->GetMetacallConfigs())
		{
			cout << "Config for " << metaConfig->GetFunctionName() << endl;
			
			string retType = metaConfig->GetReturnType();

			cout << "---params " << endl;
			for (string paramType : metaConfig->GetParametersTypes())
			{
				cout << "------" << paramType << endl;
			}
			cout << "---return type " << retType << endl;
			cout << "---------------" << endl;
		}
	}

	ScopeBase::~ScopeBase()
	{
		for (const auto &pair : this->metecalls) {
			cout << "Delete " << pair.first << endl;
			delete pair.second;
		}
	}

	IMetacall * ScopeBase::GetMetacall(string functionName) {
		IMetacall * mc = NULL;

		mc = this->metecalls[functionName];

		if (mc == NULL) {
			mc = Metacall::Make(this, functionName);
			this->metecalls[functionName] = mc;
			cout << "Create " << functionName << endl;
		}
		return mc;
	}
}
