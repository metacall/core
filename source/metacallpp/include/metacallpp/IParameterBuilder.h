#pragma once

#include <metacallpp/common.h>

#include <iostream>

namespace Beast {

	class IParameterBuilder
	{
	public:
		virtual IParameterBuilder * Param(int value) = 0;
		virtual IParameterBuilder * Param(long value) = 0;
		virtual IParameterBuilder * Param(const char * value) = 0;

		virtual ~IParameterBuilder()
		{
		}
	};
}