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

	Metacall::Metacall(IScopeBase *scope, std::string functionName)
	{
		this->functionName = functionName;
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

	std::string Metacall::GetFunctionName() {
		return this->functionName;
	}

	MetacallStates Metacall::GetState() {
		return this->state;
	}

	bool Metacall::IsReady() {
		return (this->state == MetacallStates::Ready);
	}

	Metacall * Metacall::Make(IScopeBase *scope, std::string functionName) {
		auto m = new Metacall(scope, functionName);
		m->state = MetacallStates::Ready;
		return m;
	}
}