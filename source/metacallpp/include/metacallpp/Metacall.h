/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	class METACALLPP_API Metacall :
		public IMetacall
	{
	public:
		~Metacall();

		ICall * Prepare();

		std::string GetFunctionName();

		MetacallStates GetState();

		bool IsReady();

		static Metacall * Make(IScopeBase *scope, std::string functionName);

	protected:
		Metacall(IScopeBase *scope, std::string functionName);

		std::string functionName;
		IScopeBase * scope = NULL;
		MetacallStates state = MetacallStates::UnInitialize;
	};

}

#endif /* METACALLPP_METACALL_HPP */
