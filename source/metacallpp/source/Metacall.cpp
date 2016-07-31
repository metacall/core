
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