#include <metacallpp/Return.h>

namespace Beast {
	Return::Return()
	{
	}

	Return::~Return()
	{
	}

	int Return::GetInt() {
		return 1;
	}

	void Return::Get(int *v) {
		*v = 0;
	}
	void Return::Get(std::string *v) {
		v = NULL;
	}

	std::string Return::GetString() {
		return "string";
	}
}