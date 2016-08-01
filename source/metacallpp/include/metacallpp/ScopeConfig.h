/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_SCOPE_CONFIG_HPP
#define METACALLPP_SCOPE_CONFIG_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IScopeConfig.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>
#include <metacallpp/MetacallConfig.h>

#include <list>

namespace Beast {
	class METACALLPP_API ScopeConfig :
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

#endif /* METACALLPP_SCOPE_CONFIG_HPP */
