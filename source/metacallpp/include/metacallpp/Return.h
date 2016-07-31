#pragma once

#include <metacallpp/IReturn.h>

#include <string>

namespace Beast {
	class Return :
		public IReturn
	{
	public:
		Return();
		~Return();

		int GetInt();

		void Get(int *v) ;
		void Get(std::string *v);

		std::string GetString();
	};
}


