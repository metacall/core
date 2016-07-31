#pragma once

#include <metacallpp/common.h>
#include <metacallpp/IMetacallConfigBuilder.h>

#include <list>
#include <string>

namespace Beast {

	class IMetacallConfig
	{
	public:

		virtual std::string GetFunctionName() = 0;

		virtual MetacallInvokeTypes GetInvokeType() = 0;

		virtual std::list<std::string> GetParametersTypes() = 0;

		virtual std::string GetReturnType() = 0;

		virtual IMetacallConfigBuilder* GetBuilder() = 0;

		virtual ~IMetacallConfig()
		{
		}
	};
}
