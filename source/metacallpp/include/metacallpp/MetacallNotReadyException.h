/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_METACALL_NOT_READY_EXCEPTION_HPP
#define METACALLPP_METACALL_NOT_READY_EXCEPTION_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/IMetacall.h>

#include <exception>
#include <string>
#include <sstream>

namespace Beast {


	class METACALLPP_API MetacallNotReadyException :
		public std::exception
	{
	public:

		MetacallNotReadyException(IMetacall * metacall)
		{
			this->metacall = metacall;
		}

		~MetacallNotReadyException()
		{
		}

		IMetacall * GetMetacall() {
			return this->metacall;
		}

		virtual const char* what() const throw()
		{
			return "Metacall is not ready for operation";
		}

	private:
		IMetacall * metacall = NULL;
	};
}

#endif /* METACALLPP_METACALL_NOT_READY_EXCEPTION_HPP */
