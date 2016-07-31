#pragma once

#include <metacallpp/common.h>
#include <metacallpp/ICall.h>

#include <string>

namespace Beast {

	class IMetacall
	{
	public:

		virtual ICall * Prepare() = 0;

		virtual std::string GetFunctionName() = 0;

		virtual MetacallStates GetState() = 0;

		virtual bool IsReady() = 0;

		virtual ~IMetacall()
		{
		}
	};
}
