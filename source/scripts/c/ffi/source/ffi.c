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
