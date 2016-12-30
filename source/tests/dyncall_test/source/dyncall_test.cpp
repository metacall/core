/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#include <gmock/gmock.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

typedef void (*py_loader_print_func)(void);

class dynlink_test : public testing::Test
{
  protected:
};

TEST_F(dynlink_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	dynlink_print_info();

	log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object extension: %s", dynlink_extension());

	{
		dynlink handle = dynlink_load(NULL, "py_loader", DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

		EXPECT_NE(handle, (dynlink) NULL);

		log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object file: %s", dynlink_get_name_impl(handle));

		if (handle != NULL)
		{
			static dynlink_symbol_addr py_loader_print_info_addr;

			EXPECT_EQ((int) 0, dynlink_symbol(handle, DYNLINK_SYMBOL_STR("py_loader_print_info"), &py_loader_print_info_addr));

			if (py_loader_print_info_addr != NULL)
			{
				py_loader_print_func print = DYNLINK_SYMBOL_GET(py_loader_print_info_addr);

				log_write("metacall", LOG_LEVEL_DEBUG, "Print function: %p", (void *)print);

				log_write("metacall", LOG_LEVEL_DEBUG, "Symbol pointer: %p", (void *)py_loader_print_info_addr);

				if (DYNLINK_SYMBOL_GET(py_loader_print_info_addr) != NULL)
				{
					log_write("metacall", LOG_LEVEL_DEBUG, "Pointer is valid");
				}

				print();
			}

			dynlink_unload(handle);
		}
	}
}
