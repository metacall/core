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
		dynlink handle = dynlink_load("py_loader", DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);

		EXPECT_NE(handle, (dynlink) NULL);

		if (handle != NULL)
		{
			dynlink_symbol_addr py_loader_print_info_addr;

			EXPECT_EQ((int) 0, dynlink_symbol(handle, "py_loader_print_info", &py_loader_print_info_addr));

			if (py_loader_print_info_addr != NULL)
			{
				py_loader_print_func print = DYNLINK_SYMBOL_GET(py_loader_print_info_addr);

				print();
			}

			dynlink_unload(handle);
		}
	}
}
