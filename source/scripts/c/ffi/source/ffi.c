#include <ffi.h>

typedef int yeet;

void *call_fp_address(void)
{
	return &ffi_call;
}

yeet int_type_renaming(void)
{
	return 345;
}

int c_callback(int (*sum)(int, int))
{
	return sum(3, 4);
}
