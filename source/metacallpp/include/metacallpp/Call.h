#pragma once

#include <metacallpp/ICall.h>
#include <metacallpp/IParameterBuilder.h>
#include <metacallpp/IReturn.h>
#include <metacallpp/IMetacall.h>
#include <metacallpp/CallHadInvokedException.h>
#include <metacallpp/ParameterBuilder.h>
#include <metacallpp/Return.h>

#include <iostream>
#include <string>
#include <queue>

namespace Beast {

	class Call :
		public ICall
	{
	public:
		Call(IMetacall * meta);
		IParameterBuilder * Parameters();
		IReturn * Invoke();

		~Call();
	private:
		IMetacall * meta = NULL;
		IParameterBuilder * buildParams = NULL;
		queue<void *> parameters;
	};

}
