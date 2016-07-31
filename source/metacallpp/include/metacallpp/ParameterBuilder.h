#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IParameterBuilder.h>

#include <iostream>
#include <string>
#include <queue>

namespace Beast {
	using namespace std;
class ParameterBuilder :
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

