#pragma once

#include <metacallpp/Metacall.h>
#include <metacallpp/common.h>
#include <metacallpp/IScopeBase.h>
#include <metacallpp/ICall.h>
#include <metacallpp/IReturn.h>

#include <string>

namespace Beast {
	template <typename T>
	class MetaFunction :
		public Metacall
	{
	public:
		~MetaFunction() {

		}

		T * Invoke() {
			T * v = new T();
			ICall * call = this->Prepare();
			IReturn *ret = call->Invoke();
			ret->Get(v);
			delete ret;
			delete call;
			return v;
		}

		static MetaFunction<T> * MakeFunction(IScopeBase *scope, std::string functionName) {
			MetaFunction<T> * m = new MetaFunction<T>(scope, functionName);
			m->state = MetacallStates::Ready;
			return m;
		}
	protected:
		MetaFunction(IScopeBase *scope, std::string functionName) :Metacall(scope, functionName) {

		}
	};
}