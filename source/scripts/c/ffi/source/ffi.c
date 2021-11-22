#include <ffi.h>

typedef int yeet;

void *call_fp_address()
{
	return &ffi_call;
}

yeet int_type_renaming()
{
	return 345;
}

int c_callback(int (*sum)(int, int))
{
	return sum(3, 4);
}
