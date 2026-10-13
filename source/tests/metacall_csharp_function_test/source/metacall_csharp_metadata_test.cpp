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

#include <cs_loader/defs.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

#include <cstddef>
#include <string>

#if defined(OPTION_BUILD_LOADERS_CS)

TEST(metacall_csharp_metadata_test, ManagedMetadataCapacityContract)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const std::string source = R"CS(
using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

System.Console.WriteLine("Metadata capacity contract fixture");

static int CheckMetadataCapacityContract()
{
	Assembly assembly = AppDomain.CurrentDomain.GetAssemblies()
		.Single(a => a.GetName().Name == "CSLoader");

	Type entry = assembly.GetType("CSLoader.MetacallEntryPoint", true);

	string[] exportNames = {
		"GetFunctionsV2",
		"GetClassesV2",
		"GetConstructorsV2",
		"GetMethodsV2",
		"GetAttributesV2"
	};

	string[] recordNames = {
		"ReflectFunction",
		"ReflectClass",
		"ReflectConstructor",
		"ReflectMethod",
		"ReflectAttribute"
	};

	int checkedExports = 0;

	for (int index = 0; index < exportNames.Length; ++index)
	{
		string name = exportNames[index];

		MethodInfo method = entry.GetMethod(
			name, BindingFlags.Public | BindingFlags.Static);

		if (method == null)
		{
			throw new InvalidOperationException("Missing export: " + name);
		}

		Action<bool, string> check = (condition, message) =>
		{
			if (!condition)
			{
				throw new InvalidOperationException(name + ": " + message);
			}
		};

		Func<IntPtr, int, Tuple<int, int>> invoke = (destination, capacity) =>
		{
			object[] arguments = { -1, destination, capacity };
			int status = (int)method.Invoke(null, arguments);
			return Tuple.Create(status, (int)arguments[0]);
		};

		// Status values must match metadata_status in defs.h.
		const int success = 0;
		const int insufficientCapacity = 1;
		const int invalidArgument = 3;

		var query = invoke(IntPtr.Zero, 0);

		check(query.Item1 == success, "count query failed");
		check(query.Item2 > 0, "fixture must provide non-empty metadata");

		int required = query.Item2;

		Type recordType = assembly.GetType(
			"CSLoader.MetacallDef+" + recordNames[index], true);

		int recordSize = Marshal.SizeOf(recordType);

		const int guardSize = 16;
		const byte canary = 0xA5;

		int recordBytes = checked(required * recordSize);
		int bodyBytes = checked(recordBytes + recordSize);
		int allocationBytes = checked(bodyBytes + 2 * guardSize);

		IntPtr allocation = Marshal.AllocHGlobal(allocationBytes);

		try
		{
			IntPtr destination = IntPtr.Add(allocation, guardSize);

			Action reset = () =>
			{
				for (int i = 0; i < allocationBytes; ++i)
				{
					Marshal.WriteByte(allocation, i, canary);
				}
			};

			Func<int, int, bool> untouched = (offset, length) =>
			{
				for (int i = 0; i < length; ++i)
				{
					if (Marshal.ReadByte(allocation, offset + i) != canary)
					{
						return false;
					}
				}

				return true;
			};

			reset();

			var negative = invoke(destination, -1);
			check(negative.Item1 == invalidArgument,
				"negative capacity was accepted");
			check(negative.Item2 == -1,
				"invalid argument did not preserve failure count");
			check(untouched(0, allocationBytes),
				"negative-capacity call changed destination bytes");

			var invalidNull = invoke(IntPtr.Zero, required);
			check(invalidNull.Item1 == invalidArgument,
				"null destination with positive capacity was accepted");
			check(invalidNull.Item2 == -1,
				"invalid null destination did not report failure count");

			reset();

			var zeroCapacity = invoke(destination, 0);
			check(zeroCapacity.Item1 == insufficientCapacity,
				"non-empty metadata accepted zero capacity");
			check(zeroCapacity.Item2 == required,
				"zero-capacity call reported the wrong required count");
			check(untouched(0, allocationBytes),
				"zero-capacity call wrote records");

			reset();

			var undersized = invoke(destination, required - 1);
			check(undersized.Item1 == insufficientCapacity,
				"undersized buffer was accepted");
			check(undersized.Item2 == required,
				"undersized call reported the wrong required count");
			check(untouched(0, allocationBytes),
				"undersized call changed destination bytes");

			reset();

			var exact = invoke(destination, required);
			check(exact.Item1 == success,
				"exact-capacity fill failed");
			check(exact.Item2 == required,
				"exact-capacity fill reported the wrong written count");
			check(untouched(0, guardSize),
				"exact-capacity fill changed the leading guard");
			check(untouched(guardSize + recordBytes,
				allocationBytes - guardSize - recordBytes),
				"exact-capacity fill wrote beyond the record range");

			reset();

			var oversized = invoke(destination, required + 1);
			check(oversized.Item1 == success,
				"oversized-capacity fill failed");
			check(oversized.Item2 == required,
				"oversized-capacity fill reported the wrong written count");
			check(untouched(0, guardSize),
				"oversized-capacity fill changed the leading guard");
			check(untouched(guardSize + recordBytes,
				allocationBytes - guardSize - recordBytes),
				"oversized-capacity fill wrote unused capacity or the trailing guard");

			++checkedExports;
		}
		finally
		{
			Marshal.FreeHGlobal(allocation);
		}
	}

	return checkedExports;
}

public class MetadataCapacityFixture
{
	public int value;
	public static int total;

	public MetadataCapacityFixture(int start)
	{
		value = start;
	}

	public int MetadataCapacityInstanceMethod(int delta)
	{
		return value + delta;
	}

	public static int MetadataCapacityStaticMethod(int input)
	{
		return input + 1;
	}
}
)CS";

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 source.c_str(),
					 source.size() + 1,
					 nullptr));

	void *ret = metacall("CheckMetadataCapacityContract");

	ASSERT_NE(nullptr, ret);

	EXPECT_EQ(METACALL_INT, metacall_value_id(ret));

	if (metacall_value_id(ret) == METACALL_INT)
	{
		EXPECT_EQ(5, metacall_value_to_int(ret));
	}

	metacall_value_destroy(ret);
}

TEST(metacall_csharp_metadata_test, ManagedNativeMetadataLayoutsMatch)
{
	static_assert(sizeof(int) == 4, "Metadata counts and statuses require 32-bit int");
	static_assert(sizeof(short) == 2, "Metadata type identifiers require 16-bit short");

	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	struct layout_expectation
	{
		const char *record;
		const char *field;
		size_t expected;
	};

	/* A null field means compare the complete record size. */
	const layout_expectation expectations[] = {
		{ "ReflectParam", nullptr, sizeof(reflect_param) },
		{ "ReflectParam", "type", offsetof(reflect_param, type) },
		{ "ReflectParam", "name", offsetof(reflect_param, name) },

		{ "ReflectFunction", nullptr, sizeof(reflect_function) },
		{ "ReflectFunction", "returnType", offsetof(reflect_function, return_type) },
		{ "ReflectFunction", "paramcount", offsetof(reflect_function, param_count) },
		{ "ReflectFunction", "name", offsetof(reflect_function, name) },
		{ "ReflectFunction", "pars", offsetof(reflect_function, pars) },

		{ "ReflectClass", nullptr, sizeof(reflect_class) },
		{ "ReflectClass", "name", offsetof(reflect_class, name) },

		{ "ReflectConstructor", nullptr, sizeof(reflect_constructor) },
		{ "ReflectConstructor", "className", offsetof(reflect_constructor, class_name) },
		{ "ReflectConstructor", "paramcount", offsetof(reflect_constructor, param_count) },
		{ "ReflectConstructor", "pars", offsetof(reflect_constructor, pars) },

		{ "ReflectMethod", nullptr, sizeof(reflect_method) },
		{ "ReflectMethod", "className", offsetof(reflect_method, class_name) },
		{ "ReflectMethod", "returnType", offsetof(reflect_method, return_type) },
		{ "ReflectMethod", "paramcount", offsetof(reflect_method, param_count) },
		{ "ReflectMethod", "name", offsetof(reflect_method, name) },
		{ "ReflectMethod", "isStatic", offsetof(reflect_method, is_static) },
		{ "ReflectMethod", "pars", offsetof(reflect_method, pars) },

		{ "ReflectAttribute", nullptr, sizeof(reflect_attribute) },
		{ "ReflectAttribute", "className", offsetof(reflect_attribute, class_name) },
		{ "ReflectAttribute", "type", offsetof(reflect_attribute, type) },
		{ "ReflectAttribute", "name", offsetof(reflect_attribute, name) },
		{ "ReflectAttribute", "isStatic", offsetof(reflect_attribute, is_static) }
	};

	const size_t expectation_count =
		sizeof(expectations) / sizeof(expectations[0]);

	std::string source = R"CS(
using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

System.Console.WriteLine("Native and managed metadata layout fixture");

static int CheckMetadataLayouts()
{
	Assembly assembly = AppDomain.CurrentDomain.GetAssemblies()
		.Single(a => a.GetName().Name == "CSLoader");

	int checkedLayouts = 0;

	Action<string, string, int> check = (recordName, fieldName, expected) =>
	{
		Type recordType = assembly.GetType(
			"CSLoader.MetacallDef+" + recordName, true);

		int actual = fieldName == null
			? Marshal.SizeOf(recordType)
			: Marshal.OffsetOf(recordType, fieldName).ToInt32();

		if (actual != expected)
		{
			throw new InvalidOperationException(
				recordName + "." + (fieldName ?? "sizeof") +
				": native=" + expected + ", managed=" + actual);
		}

		++checkedLayouts;
	};
)CS";

	for (const layout_expectation &expectation : expectations)
	{
		source += "    check(\"";
		source += expectation.record;
		source += "\", ";

		if (expectation.field == nullptr)
		{
			source += "null";
		}
		else
		{
			source += "\"";
			source += expectation.field;
			source += "\"";
		}

		source += ", ";
		source += std::to_string(expectation.expected);
		source += ");\n";
	}

	source += R"CS(
	return checkedLayouts;
}
)CS";

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 source.c_str(),
					 source.size() + 1,
					 nullptr));

	void *ret = metacall("CheckMetadataLayouts");

	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(METACALL_INT, metacall_value_id(ret));

	if (metacall_value_id(ret) == METACALL_INT)
	{
		EXPECT_EQ(static_cast<int>(expectation_count), metacall_value_to_int(ret));
	}

	metacall_value_destroy(ret);
}

TEST(metacall_csharp_metadata_test, ManagedMetadataEmptyAndFailureContract)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const std::string source = R"CS(
using System;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

System.Console.WriteLine("Empty and failed metadata writer fixture");

static int CheckEmptyAndFailedMetadata()
{
	Assembly assembly = AppDomain.CurrentDomain.GetAssemblies()
		.Single(a => a.GetName().Name == "CSLoader");

	Type entry = assembly.GetType("CSLoader.MetacallEntryPoint", true);

	MethodInfo helper = entry.GetMethod(
		"WriteMetadata", BindingFlags.NonPublic | BindingFlags.Static);

	if (helper == null)
	{
		throw new InvalidOperationException("Missing WriteMetadata helper");
	}

	MethodInfo writer = helper.MakeGenericMethod(typeof(int));

	Action<bool, string> check = (condition, message) =>
	{
		if (!condition)
		{
			throw new InvalidOperationException(message);
		}
	};

	Func<Func<int[]>, IntPtr, int, Tuple<int, int>> invoke =
		(producer, destination, capacity) =>
		{
			object[] arguments = { producer, -1, destination, capacity };
			int status = (int)writer.Invoke(null, arguments);
			return Tuple.Create(status, (int)arguments[1]);
		};

	const int success = 0;
	const int failure = 2;
	const byte canary = 0xA5;
	const int bufferBytes = 16;

	Func<int[]> empty = () => Array.Empty<int>();

	Func<int[]> throwing = () =>
	{
		throw new InvalidOperationException("Injected metadata producer failure");
	};

	IntPtr buffer = Marshal.AllocHGlobal(bufferBytes);

	try
	{
		for (int i = 0; i < bufferBytes; ++i)
		{
			Marshal.WriteByte(buffer, i, canary);
		}

		Func<bool> untouched = () =>
		{
			for (int i = 0; i < bufferBytes; ++i)
			{
				if (Marshal.ReadByte(buffer, i) != canary)
				{
					return false;
				}
			}

			return true;
		};

		var emptyQuery = invoke(empty, IntPtr.Zero, 0);
		check(emptyQuery.Item1 == success, "empty query failed");
		check(emptyQuery.Item2 == 0, "empty query did not return count zero");

		var emptyFill = invoke(empty, buffer, 1);
		check(emptyFill.Item1 == success, "empty fill failed");
		check(emptyFill.Item2 == 0, "empty fill did not return count zero");
		check(untouched(), "empty fill changed destination bytes");

		var failedQuery = invoke(throwing, IntPtr.Zero, 0);
		check(failedQuery.Item1 == failure,
			"producer exception was not converted to failure status");
		check(failedQuery.Item2 == -1,
			"failed query did not return failure count");

		var failedFill = invoke(throwing, buffer, 1);
		check(failedFill.Item1 == failure,
			"failed fill did not return failure status");
		check(failedFill.Item2 == -1,
			"failed fill did not return failure count");
		check(untouched(), "failed producer changed destination bytes");

		return 4;
	}
	finally
	{
		Marshal.FreeHGlobal(buffer);
	}
}
)CS";

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 source.c_str(),
					 source.size() + 1,
					 nullptr));

	void *ret = metacall("CheckEmptyAndFailedMetadata");

	ASSERT_NE(nullptr, ret);
	EXPECT_EQ(METACALL_INT, metacall_value_id(ret));

	if (metacall_value_id(ret) == METACALL_INT)
	{
		EXPECT_EQ(4, metacall_value_to_int(ret));
	}

	metacall_value_destroy(ret);
}
#endif /* OPTION_BUILD_LOADERS_CS */
