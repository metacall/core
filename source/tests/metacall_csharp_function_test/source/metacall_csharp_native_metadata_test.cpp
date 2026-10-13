/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <cs_loader/netcore.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

#include <new>

#if defined(OPTION_BUILD_LOADERS_CS)

TEST(metacall_csharp_native_metadata_test, NativeMetadataRetryAndFailureContract)
{
	/* Initialize logging and MetaCall, but do not load the C# runtime. */
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	class fake_netcore : public netcore
	{
	public:
		fake_netcore() :
			netcore(nullptr, nullptr)
		{
			this->core_get_functions = nullptr;
		}

		bool start() override
		{
			return false;
		}

		void stop() override
		{
		}

		bool create_delegate(const CHARSTRING *, void **) override
		{
			return false;
		}

		size_t published_size() const
		{
			return this->functions.size();
		}

		int published_parameter_count() const
		{
			return this->functions.empty() ? -1 : this->functions[0].param_count;
		}
	};

	fake_netcore core;
	int count = 123;

	/* First publish a known successful buffer. */
	core.core_get_functions = +[](int *out, reflect_function *destination, int capacity) -> int {
		*out = 1;

		if (destination == nullptr)
		{
			return METADATA_SUCCESS;
		}

		if (capacity < 1)
		{
			return METADATA_INSUFFICIENT_CAPACITY;
		}

		destination[0] = reflect_function{};
		destination[0].param_count = 7;
		return METADATA_SUCCESS;
	};

	ASSERT_NE(nullptr, core.get_functions(&count));
	ASSERT_EQ(1, count);
	ASSERT_EQ(1u, core.published_size());
	ASSERT_EQ(7, core.published_parameter_count());

	/* A failed fill must discard partially written temporary metadata. */
	core.core_get_functions = +[](int *out, reflect_function *destination, int) -> int {
		*out = 2;

		if (destination == nullptr)
		{
			return METADATA_SUCCESS;
		}

		destination[0] = reflect_function{};
		destination[0].param_count = 99;
		*out = -1;
		return METADATA_FAILURE;
	};

	count = 123;
	EXPECT_EQ(nullptr, core.get_functions(&count));
	EXPECT_EQ(-1, count);
	EXPECT_EQ(1u, core.published_size());
	EXPECT_EQ(7, core.published_parameter_count());

	/* A callback exception must not escape the native getter. */
	core.core_get_functions = +[](int *, reflect_function *, int) -> int {
		throw std::bad_alloc();
	};

	count = 123;
	EXPECT_NO_THROW({
		EXPECT_EQ(nullptr, core.get_functions(&count));
	});

	EXPECT_EQ(-1, count);
	EXPECT_EQ(1u, core.published_size());
	EXPECT_EQ(7, core.published_parameter_count());

	/* Repeated growth must stop after the bounded number of attempts. */
	static int query_calls = 0;
	static int fill_calls = 0;
	query_calls = 0;
	fill_calls = 0;

	core.core_get_functions = +[](int *out, reflect_function *destination, int capacity) -> int {
		if (destination == nullptr)
		{
			++query_calls;
			*out = 1;
			return METADATA_SUCCESS;
		}

		++fill_calls;
		*out = capacity + 1;
		return METADATA_INSUFFICIENT_CAPACITY;
	};

	count = 123;
	EXPECT_EQ(nullptr, core.get_functions(&count));
	EXPECT_EQ(-1, count);
	EXPECT_EQ(3, query_calls);
	EXPECT_EQ(3, fill_calls);
	EXPECT_EQ(1u, core.published_size());
	EXPECT_EQ(7, core.published_parameter_count());

	/* One growth event followed by a successful fill must recover. */
	query_calls = 0;
	fill_calls = 0;

	core.core_get_functions = +[](int *out, reflect_function *destination, int capacity) -> int {
		if (destination == nullptr)
		{
			++query_calls;
			*out = query_calls == 1 ? 1 : 2;
			return METADATA_SUCCESS;
		}

		++fill_calls;

		if (capacity < 2)
		{
			*out = 2;
			return METADATA_INSUFFICIENT_CAPACITY;
		}

		destination[0] = reflect_function{};
		destination[1] = reflect_function{};
		destination[0].param_count = 8;
		destination[1].param_count = 9;
		*out = 2;
		return METADATA_SUCCESS;
	};

	count = 123;
	reflect_function *records = core.get_functions(&count);

	ASSERT_NE(nullptr, records);
	ASSERT_EQ(2, count);
	EXPECT_EQ(2, query_calls);
	EXPECT_EQ(2, fill_calls);
	EXPECT_EQ(8, records[0].param_count);
	EXPECT_EQ(9, records[1].param_count);

	/* Empty success must remain distinguishable from failure. */
	core.core_get_functions = +[](int *out, reflect_function *, int) -> int {
		*out = 0;
		return METADATA_SUCCESS;
	};

	count = 123;
	EXPECT_EQ(nullptr, core.get_functions(&count));
	EXPECT_EQ(0, count);
	EXPECT_EQ(0u, core.published_size());

	/* Reject an invalid successful-query count. */
	core.core_get_functions = +[](int *out, reflect_function *, int) -> int {
		*out = -1;
		return METADATA_SUCCESS;
	};

	count = 123;
	EXPECT_EQ(nullptr, core.get_functions(&count));
	EXPECT_EQ(-1, count);
}
#endif /* OPTION_BUILD_LOADERS_CS */
