/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_ISCOPE_CONFIG_HPP
#define METACALLPP_ISCOPE_CONFIG_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IMetacallConfig.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>

namespace Beast {
	class METACALLPP_API IScopeConfig
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

#endif /* METACALLPP_ISCOPE_CONFIG_HPP */
