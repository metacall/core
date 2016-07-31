#pragma once

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
