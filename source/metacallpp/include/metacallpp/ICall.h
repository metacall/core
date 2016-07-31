#pragma once

#include <metacallpp/IParameterBuilder.h>
#include <metacallpp/IReturn.h>

#include <iostream>

namespace Beast {
	using namespace std;

	class ICall
	{
	public:

		virtual IParameterBuilder * Parameters() = 0;
		 
		virtual IReturn * Invoke() = 0;

		virtual ~ICall() {
		}
	};
}
