/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <dynlink/dynlink.h>

#include <cstdio>

typedef void (*py_loader_print_func)(void);

class dynlink_test : public testing::Test
{
  protected:
};

TEST_F(dynlink_test, DefaultConstructor)
{
	dynlink_print_info();

	printf("Dynamic linked shared object extension: %s\n", dynlink_extension());

	{
		dynlink handle = dynlink_load("py_loader", DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

		EXPECT_NE(handle, (dynlink) NULL);

		printf("Dynamic linked shared object file: %s\n", dynlink_get_name_impl(handle));

		if (handle != NULL)
		{
			static dynlink_symbol_addr py_loader_print_info_addr;

			EXPECT_EQ((int) 0, dynlink_symbol(handle, DYNLINK_SYMBOL_STR("py_loader_print_info"), &py_loader_print_info_addr));

			if (py_loader_print_info_addr != NULL)
			{
				py_loader_print_func print = DYNLINK_SYMBOL_GET(py_loader_print_info_addr);

				printf("Print function: %p\n", (void *)print);

				printf("Symbol pointer: %p\n", (void *)py_loader_print_info_addr);

				if (DYNLINK_SYMBOL_GET(py_loader_print_info_addr) != NULL)
				{
					printf("Pointer is valid\n");
				}

				print();
			}

			dynlink_unload(handle);
		}
	}
}
