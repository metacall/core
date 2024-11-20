#include "loadtest.h"
#include <vector>

long call_cpp_func(void)
{
	std::vector<int> v = { 7, 323, 14, 8 };

	return v[1];
}

int pair_list_init(pair_list **t)
{
	static const uint32_t size = 3;

	*t = new pair_list();

	(*t)->size = size;
	(*t)->pairs = new pair[(*t)->size];

	for (uint32_t i = 0; i < size; ++i)
	{
		(*t)->pairs[i].i = i;
		(*t)->pairs[i].d = (double)(((double)i) * 1.0);
	}

	return 0;
}

double pair_list_value(pair_list *t, uint32_t id)
{
	return t->pairs[id].d;
}

void pair_list_destroy(pair_list *t)
{
	delete[] t->pairs;
	delete t;
}

void modify_int_ptr(long *l)
{
	*l = 111;
}
