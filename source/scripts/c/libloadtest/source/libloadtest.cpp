#include "libloadtest.h"
#include <vector>

long call_cpp_func(void)
{
	std::vector<int> v = { 7, 323, 16, 8 };

	return v[1];
}
