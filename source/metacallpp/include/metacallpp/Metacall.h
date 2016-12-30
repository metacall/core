/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_METACALL_HPP
#define METACALLPP_METACALL_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IScopeBase.h>
#include <metacallpp/IMetacall.h>
#include <metacallpp/ICall.h>
#include <metacallpp/MetacallNotReadyException.h>
#include <metacallpp/Call.h>

#include <iostream>
#include <string>

namespace Beast {
	class Metacall :
		public IMetacall
	{
	public:
		METACALLPP_API virtual ~Metacall();

		METACALLPP_API virtual ICall * Prepare();

		METACALLPP_API virtual const char * GetFunctionName();

		METACALLPP_API virtual MetacallStates GetState();

		METACALLPP_API virtual bool IsReady();

		METACALLPP_API static Metacall * Make(IScopeBase *scope, const char * functionName);

	protected:
		METACALLPP_API Metacall(IScopeBase *scope, const char * functionName);

		std::string functionName;
		IScopeBase * scope = NULL;
		MetacallStates state = MetacallStates::UnInitialize;
	};

}

#endif /* METACALLPP_METACALL_HPP */
