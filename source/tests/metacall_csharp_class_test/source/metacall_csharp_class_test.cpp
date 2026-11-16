/*
 * MetaCall Library by Parra Studios
 * A library for providing a foreign function interface calls.
 *
 * Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

class metacall_csharp_class_test : public testing::Test
{
public:
};

TEST_F(metacall_csharp_class_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* C# Netcore */
#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char *cs_scripts[] = {
			"class.cs"
		};

		ASSERT_EQ((int)0, (int)metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));

		/* Get the class handle */
		void *myclass = metacall_class("Counter");
		ASSERT_NE((void *)NULL, (void *)myclass);

		/* Construct: new Counter(5) */
		void *constructor_params[] = {
			metacall_value_create_int(5)
		};

		void *new_object_v = metacall_class_new(myclass, "Counter", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
		ASSERT_NE((void *)NULL, (void *)new_object_v);

		void *new_object = metacall_value_to_object(new_object_v);

		/* Read the instance attribute: count == 5 */
		{
			void *count = metacall_object_get(new_object, "count");
			EXPECT_EQ((int)5, (int)metacall_value_to_int(count));
			metacall_value_destroy(count);
		}

		/* Invoke the instance method: Add(7) == 12 */
		{
			void *args[] = {
				metacall_value_create_int(7)
			};

			void *ret = metacallt_object(new_object, "Add", METACALL_INT, args, sizeof(args) / sizeof(args[0]));
			EXPECT_EQ((int)12, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		/* Invoke the static method: Counter.Twice(21) == 42 */
		{
			void *args[] = {
				metacall_value_create_int(21)
			};

			void *ret = metacallt_class(myclass, "Twice", METACALL_INT, args, sizeof(args) / sizeof(args[0]));
			EXPECT_EQ((int)42, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		/* Static attribute: Counter.total set/get == 99 */
		{
			void *val = metacall_value_create_int(99);
			ASSERT_EQ((int)0, (int)metacall_class_static_set(myclass, "total", val));
			metacall_value_destroy(val);

			void *ret = metacall_class_static_get(myclass, "total");
			EXPECT_EQ((int)99, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		metacall_value_destroy(new_object_v);
	}
#endif /* OPTION_BUILD_LOADERS_CS */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	metacall_destroy();
}

#if defined(OPTION_BUILD_LOADERS_CS)
TEST_F(metacall_csharp_class_test, MoreThan100ClassMetadataRecords)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	using owned_value = std::unique_ptr<void, decltype(&metacall_value_destroy)>;

	auto expect_integer_result = [](void *raw, int expected) {
		owned_value ret(raw, &metacall_value_destroy);

		ASSERT_NE(nullptr, ret.get());
		EXPECT_EQ(METACALL_INT, metacall_value_id(ret.get()));

		if (metacall_value_id(ret.get()) == METACALL_INT)
		{
			EXPECT_EQ(expected, metacall_value_to_int(ret.get()));
		}
	};

	const int class_count = 101;
	std::string source;

	for (int i = 0; i < class_count; ++i)
	{
		const std::string suffix = std::to_string(i);
		const std::string class_name = "MetadataBoundaryClass" + suffix;

		source +=
			"public class " + class_name + "\n"
										   "{\n"
										   "    public int value;\n"
										   "    public static int total = " +
			suffix + ";\n"
					 "    public " +
			class_name + "(int start) { value = start; }\n"
						 "    public int MetadataInstanceMethod" +
			suffix +
			"(int delta) { return value + delta; }\n"
			"    public static int MetadataStaticMethod" +
			suffix +
			"(int input) { return input + " + suffix + "; }\n"
													   "}\n";
	}

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 source.c_str(),
					 source.size() + 1,
					 nullptr));

	for (int i = 0; i < class_count; ++i)
	{
		SCOPED_TRACE(i);

		const std::string suffix = std::to_string(i);
		const std::string class_name = "MetadataBoundaryClass" + suffix;

		void *cls = metacall_class(class_name.c_str());
		ASSERT_NE(nullptr, cls);

		/* Exercise this class's constructor. */
		owned_value start(metacall_value_create_int(i), &metacall_value_destroy);
		ASSERT_NE(nullptr, start.get());

		void *constructor_args[] = { start.get() };

		owned_value object_value(
			metacall_class_new(cls, class_name.c_str(), constructor_args, 1),
			&metacall_value_destroy);

		ASSERT_NE(nullptr, object_value.get());
		ASSERT_EQ(METACALL_OBJECT, metacall_value_id(object_value.get()));

		void *obj = metacall_value_to_object(object_value.get());
		ASSERT_NE(nullptr, obj);

		/* Exercise the instance attribute exported for this class. */
		expect_integer_result(metacall_object_get(obj, "value"), i);

		owned_value updated(
			metacall_value_create_int(1000 + i),
			&metacall_value_destroy);

		ASSERT_NE(nullptr, updated.get());
		ASSERT_EQ(0, metacall_object_set(obj, "value", updated.get()));

		expect_integer_result(metacall_object_get(obj, "value"), 1000 + i);

		/* Exercise both method kinds using their unique names. */
		owned_value input(metacall_value_create_int(7), &metacall_value_destroy);
		ASSERT_NE(nullptr, input.get());

		void *method_args[] = { input.get() };

		const std::string instance_method = "MetadataInstanceMethod" + suffix;

		expect_integer_result(
			metacallt_object(obj, instance_method.c_str(), METACALL_INT, method_args, 1),
			1007 + i);

		const std::string static_method = "MetadataStaticMethod" + suffix;

		expect_integer_result(
			metacallt_class(cls, static_method.c_str(), METACALL_INT, method_args, 1),
			7 + i);

		/* Exercise the static attribute exported for this class. */
		expect_integer_result(metacall_class_static_get(cls, "total"), i);

		ASSERT_EQ(0, metacall_class_static_set(cls, "total", updated.get()));

		expect_integer_result(metacall_class_static_get(cls, "total"), 1000 + i);
	}
}
#endif /* OPTION_BUILD_LOADERS_CS */

#if defined(OPTION_BUILD_LOADERS_CS)
TEST_F(metacall_csharp_class_test, EmptyClass)
{
	ASSERT_EQ(0, metacall_initialize());

	/* Always shut down, including after a fatal assertion. */
	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const char source[] = "public class DiscoveryEmptyClass { }";

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs", source, sizeof(source), nullptr));

	ASSERT_NE(nullptr, metacall_class("DiscoveryEmptyClass"));
}

TEST_F(metacall_csharp_class_test, ConstructorWith10Parameters)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const char source[] =
		"public class ConstructorBoundary10 {"
		"  public int value;"
		"  public ConstructorBoundary10("
		"    int p0, int p1, int p2, int p3, int p4,"
		"    int p5, int p6, int p7, int p8, int p9)"
		"  { value = p9; }"
		"}";

	ASSERT_EQ(0, metacall_load_from_memory("cs", source, sizeof(source), nullptr));

	void *cls = metacall_class("ConstructorBoundary10");
	ASSERT_NE(nullptr, cls);

	struct argument_cleanup
	{
		void *values[10] = {};

		~argument_cleanup()
		{
			for (void *value : values)
			{
				if (value != nullptr)
				{
					metacall_value_destroy(value);
				}
			}
		}
	} args;

	for (int i = 0; i < 10; ++i)
	{
		args.values[i] = metacall_value_create_int(i);
		ASSERT_NE(nullptr, args.values[i]);
	}

	using owned_value = std::unique_ptr<void, decltype(&metacall_value_destroy)>;

	owned_value object_value(
		metacall_class_new(cls, "ConstructorBoundary10", args.values, 10),
		&metacall_value_destroy);

	ASSERT_NE(nullptr, object_value.get());
	ASSERT_EQ(METACALL_OBJECT, metacall_value_id(object_value.get()));

	void *obj = metacall_value_to_object(object_value.get());
	ASSERT_NE(nullptr, obj);

	owned_value value(metacall_object_get(obj, "value"), &metacall_value_destroy);
	ASSERT_NE(nullptr, value.get());
	ASSERT_EQ(METACALL_INT, metacall_value_id(value.get()));
	EXPECT_EQ(9, metacall_value_to_int(value.get()));
}

TEST_F(metacall_csharp_class_test, ConstructorWith11ParametersProbe)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const char source[] =
		"public class ConstructorBoundary11 {"
		"  public ConstructorBoundary11("
		"    int p0, int p1, int p2, int p3, int p4,"
		"    int p5, int p6, int p7, int p8, int p9, int p10)"
		"  { }"
		"}";

	EXPECT_NE(0, metacall_load_from_memory("cs", source, sizeof(source), nullptr));
}

TEST_F(metacall_csharp_class_test, StaticClassWithoutConstructors)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const char source[] =
		"public static class DiscoveryStaticClass {"
		"  public static int Twice(int value) { return value * 2; }"
		"}";

	ASSERT_EQ(0, metacall_load_from_memory("cs", source, sizeof(source), nullptr));

	void *cls = metacall_class("DiscoveryStaticClass");
	ASSERT_NE(nullptr, cls);

	using owned_value = std::unique_ptr<void, decltype(&metacall_value_destroy)>;

	owned_value input(metacall_value_create_int(21), &metacall_value_destroy);
	ASSERT_NE(nullptr, input.get());

	void *args[] = { input.get() };

	owned_value result(
		metacallt_class(cls, "Twice", METACALL_INT, args, 1),
		&metacall_value_destroy);

	ASSERT_NE(nullptr, result.get());
	ASSERT_EQ(METACALL_INT, metacall_value_id(result.get()));
	EXPECT_EQ(42, metacall_value_to_int(result.get()));
}
#endif
