/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_ICALL_HPP
#define METACALLPP_ICALL_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/IParameterBuilder.h>
#include <metacallpp/IReturn.h>

#include <iostream>

namespace Beast {
	using namespace std;

	class METACALLPP_API ICall
	{
	public:

		virtual IParameterBuilder * Parameters() = 0;
		 
		virtual IReturn * Invoke() = 0;

		virtual ~ICall() {
		}
	};
}

#endif /* METACALLPP_ICALL_HPP */
