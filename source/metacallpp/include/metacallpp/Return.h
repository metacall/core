/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_RETURN_HPP
#define METACALLPP_RETURN_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/IReturn.h>

#include <string>

namespace Beast {
	class METACALLPP_API Return :
		public IReturn
	{
	public:
		Return();
		~Return();

		int GetInt();

		void Get(int *v) ;
		void Get(std::string *v);

		std::string GetString();
	};
}

#endif /* METACALLPP_RETURN_HPP */
