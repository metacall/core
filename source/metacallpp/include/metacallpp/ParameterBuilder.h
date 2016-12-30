/*
*	MetaCall++ Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*	Copyright (C) 2016 Federico Agustin Alvarez Bayon <loopzer@gmail.com>
*
*	High performance, type safe and exception safe object oriented
*	front-end for MetaCall library.
*
*/

#ifndef METACALLPP_PARAMETER_BUILDER_HPP
#define METACALLPP_PARAMETER_BUILDER_HPP 1

/* -- Headers -- */

#include <metacallpp/metacallpp_api.h>

#include <metacallpp/common.h>
#include <metacallpp/IParameterBuilder.h>

#include <iostream>
#include <string>
#include <queue>

namespace Beast {
	using namespace std;
class METACALLPP_API ParameterBuilder :
	public IParameterBuilder
{
public:
	ParameterBuilder(queue<void *> * parameters);

	IParameterBuilder * Param(int value);
	
	IParameterBuilder * Param(long value);

	IParameterBuilder * Param(const char * value);

	~ParameterBuilder();
protected:
	queue<void *> * parameters;
};

}

#endif /* METACALLPP_PARAMETER_BUILDER_HPP */
