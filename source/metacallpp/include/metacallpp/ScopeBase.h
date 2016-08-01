/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_SCOPE_BASE_HPP
#define METACALLPP_SCOPE_BASE_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/IMetacall.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/Metacall.h>
#include <metacallpp/MetaFunction.h>

#include <iostream>
#include <string>
#include <map>

namespace Beast {
	class METACALLPP_API ScopeBase :
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

#endif /* METACALLPP_SCOPE_BASE_HPP */
