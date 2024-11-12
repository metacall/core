#include "loadtest.h"
#include <vector>

long call_cpp_func(void)
{
	std::vector<int> v = { 7, 323, 14, 8 };

	return v[1];
}
