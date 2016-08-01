/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_IRETURN_HPP
#define METACALLPP_IRETURN_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <string>

namespace Beast {
	class METACALLPP_API IReturn
	{
	public:
		virtual int GetInt() = 0;

		virtual std::string GetString() = 0;

		virtual void Get(int *v) = 0;
		virtual void Get(std::string *v) = 0;

		virtual ~IReturn()
		{
		}
	};
}

#endif /* METACALLPP_IRETURN_HPP */
