#pragma once

#include <metacallpp/IMetacall.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/Metacall.h>
#include <metacallpp/MetaFunction.h>

#include <iostream>
#include <string>
#include <map>

namespace Beast {
	class ScopeBase :
		public IScopeBase
	{
	public:
		ScopeBase(IScopeConfig * config);
		~ScopeBase();
		IMetacall * GetMetacall(std::string functionName);

		template <typename T>
		MetaFunction<T> * GetFunc(std::string functionName) {
			MetaFunction<T>  * mc = NULL;

			mc = (MetaFunction<T>*)this->metecalls[functionName];

			if (mc == NULL) {
				mc = MetaFunction<T>::MakeFunction(this, functionName);
				this->metecalls[functionName] = mc;
				cout << "Create " << functionName << endl;
			}
			return mc;
		}
	private:
		IScopeConfig * config = NULL;
		std::map<std::string, IMetacall*> metecalls;
	};
}