#pragma once

#include <metacallpp/ICall.h>

#include <exception>
#include <string>
#include <sstream>

namespace Beast {
	class CallHadInvokedException :
		public std::exception
	{
	public:

		CallHadInvokedException(ICall * call)
		{
			this->call = call;
		}

		~CallHadInvokedException()
		{
		}

		ICall * GetCall() {
			return this->call;
		}

		virtual const char* what() const throw()
		{
			return "Call it Had Invoked";
		}

	private:
		ICall * call = NULL;
	};
}

