#pragma once

#include <metacallpp/IMetacall.h>

#include <exception>
#include <string>
#include <sstream>

namespace Beast {


	class MetacallNotReadyException :
		public std::exception
	{
	public:

		MetacallNotReadyException(IMetacall * metacall)
		{
			this->metacall = metacall;
		}

		~MetacallNotReadyException()
		{
		}

		IMetacall * GetMetacall() {
			return this->metacall;
		}

		virtual const char* what() const throw()
		{
			return "Metacall is not ready for operation";
		}

	private:
		IMetacall * metacall = NULL;
	};
}

