/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	class ScopeBase :
		public IScopeBase
	{
	public:
		METACALLPP_API ScopeBase(IScopeConfig * config);
		METACALLPP_API ~ScopeBase();

		METACALLPP_API IMetacall * GetMetacall(const char * functionName);

		/* TODO: Move this to a inl file */
		template <typename T>
		MetaFunction<T> * GetFunc(const char * functionName) {
			MetaFunction<T>  * mc = NULL;

			std::string name_str = std::string(functionName);

			mc = (MetaFunction<T>*)this->metecalls[name_str];

			if (mc == NULL) {
				mc = MetaFunction<T>::MakeFunction(this, functionName);
				this->metecalls[name_str] = mc;
				cout << "Create " << name_str << endl;
			}
			return mc;
		}
	private:
		IScopeConfig * config = NULL;
		std::map<std::string, IMetacall*> metecalls;
	};
}

#endif /* METACALLPP_SCOPE_BASE_HPP */
