/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

/* -- Headers -- */

#include <metacallpp/Return.h>

namespace Beast {
	Return::Return()
	{
	}

	Return::~Return()
	{
	}

	int Return::GetInt() {
		return 1;
	}

	void Return::Get(int *v) {
		*v = 0;
	}
	void Return::Get(std::string *v) {
		(void)v;
	}

	std::string Return::GetString() {
		return "string";
	}
}
