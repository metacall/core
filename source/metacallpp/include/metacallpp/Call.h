/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_CALL_HPP
#define METACALLPP_CALL_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

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

	class METACALLPP_API Call :
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

#endif /* METACALLPP_CALL_HPP */
