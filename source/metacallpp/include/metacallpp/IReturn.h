#pragma once

#include <string>

namespace Beast {
	class IReturn
	{
	public:
		virtual int GetInt() = 0;

		virtual std::string GetString() = 0;

		virtual void Get(int *v) = 0;
		virtual void Get(std::string *v) = 0;

		virtual ~IReturn()
		{
		}
	};
}
