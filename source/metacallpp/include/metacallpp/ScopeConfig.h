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

	class ScopeConfig :
		public IScopeConfig

	{
	public:

		METACALLPP_API ScopeConfig(MetacallInvokeTypes type);

		METACALLPP_API MetacallInvokeTypes GetMetacallType();

		METACALLPP_API IMetacallConfigBuilder * CreateMetacallConfig();

		/* TODO: Remove STD from class interface in order to be exportable */
		std::list<IMetacallConfig *>* GetMetacallConfigs();

		METACALLPP_API ~ScopeConfig();

	private:
		MetacallInvokeTypes metacallInvokeType;

		std::list<IMetacallConfig *> metacallConfigs;
	};
}

#endif /* METACALLPP_SCOPE_CONFIG_HPP */
