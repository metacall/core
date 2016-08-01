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

#include <metacallpp/ParameterBuilder.h>

namespace Beast {

	ParameterBuilder::ParameterBuilder(queue<void *> * parameters)
	{
		this->parameters = parameters;
	}

	IParameterBuilder * ParameterBuilder::Param(int value) {
		cout << "int:" << value << endl;
		this->parameters->push(&value);
		return this;
	}

	IParameterBuilder * ParameterBuilder::Param(long value) {
		cout << "long:" << value << endl;
		this->parameters->push(&value);
		return this;
	}

	IParameterBuilder * ParameterBuilder::Param(const char * value) {
		cout << "const char *:" << value << endl;
		this->parameters->push((void*)value);
		return this;
	}

	ParameterBuilder::~ParameterBuilder()
	{
	}
}
