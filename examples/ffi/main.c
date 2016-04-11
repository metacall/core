#include <stdio.h>
#include <ffi.h>

int main(int argc, char *argv[])
{
	ffi_cif cif;
	ffi_type *args[1];
	void *values[1];
	char *str;
	int result;

	args[0] = &ffi_type_pointer;
	values[0] = &str;

	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_uint, args) == FFI_OK)
	{
		str = "Hello World!";

		ffi_call(&cif, (void(*)(void))puts, &result, values);

		str = "Cool!";

		ffi_call(&cif, (void(*)(void))puts, &result, values);
	}

	return 0;
}
