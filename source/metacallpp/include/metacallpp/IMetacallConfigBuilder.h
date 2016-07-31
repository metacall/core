#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IMetacall.h>

#include <string>

namespace Beast {

	class IMetacallConfigBuilder
	{
	public:

		virtual IMetacallConfigBuilder* SetFunctionName(std::string) = 0;

		virtual IMetacallConfigBuilder* SetInvokeType(MetacallInvokeTypes) = 0;
		
		virtual IMetacallConfigBuilder* SetParameterType(std::string) = 0;
		
		virtual IMetacallConfigBuilder* SetReturnType(std::string) = 0;

		virtual ~IMetacallConfigBuilder()
		{
		}
	};
}
