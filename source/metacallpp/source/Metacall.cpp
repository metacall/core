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

#include <metacallpp/Metacall.h>

namespace Beast {

	Metacall::Metacall(IScopeBase *scope, const char * functionName)
	{
		this->functionName = std::string(functionName);
		this->scope = scope;
	}

	Metacall::~Metacall()
	{
	}

	ICall * Metacall::Prepare() {

		if (!this->IsReady()) {
			throw new MetacallNotReadyException(this);
		}
		else
		{
			return new Call(this);
		}
	}

	const char * Metacall::GetFunctionName() {
		return this->functionName.c_str();
	}

	MetacallStates Metacall::GetState() {
		return this->state;
	}

	bool Metacall::IsReady() {
		return (this->state == MetacallStates::Ready);
	}

	Metacall * Metacall::Make(IScopeBase *scope, const char * functionName) {
		auto m = new Metacall(scope, functionName);
		m->state = MetacallStates::Ready;
		return m;
	}
}