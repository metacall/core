/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_CALL_HAD_INVOKED_EXCEPTION_HPP
#define METACALLPP_CALL_HAD_INVOKED_EXCEPTION_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/ICall.h>

#include <exception>
#include <string>
#include <sstream>

namespace Beast {
	class METACALLPP_API CallHadInvokedException :
		public std::exception
	{
	public:

		CallHadInvokedException(ICall * call)
		{
			this->call = call;
		}

		~CallHadInvokedException()
		{
		}

		ICall * GetCall() {
			return this->call;
		}

		virtual const char* what() const throw()
		{
			return "Call it Had Invoked";
		}

	private:
		ICall * call = NULL;
	};
}

#endif /* METACALLPP_CALL_HAD_INVOKED_EXCEPTION_HPP */
