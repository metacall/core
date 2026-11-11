/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rust code at run-time into a process.
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

#include <metacall/metacall.hpp>

#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

using namespace metacall;

/* Templates to instantiate at load time, each element is { "<template>": [ "<type>", ... ] } */
using template_map = metacall::map_typed<std::string, metacall::array>;

class metacall_rust_reinitialize_test : public testing::Test
{
protected:
	static void SetUpTestSuite()
	{
	}

	static void TearDownTestSuite()
	{
	}
};

/* Reads a script of the scripts folder for loading it from memory */
static std::string script_read(const char *name)
{
	const char *path = std::getenv("LOADER_SCRIPT_PATH");
	std::ifstream file(std::string(path != NULL ? path : ".") + "/" + name);
	std::stringstream buffer;

	buffer << file.rdbuf();

	return buffer.str();
}

static int call_int(const char *name)
{
	void *ret = metacall::metacall(name);

	EXPECT_NE((void *)NULL, ret);

	if (ret == NULL)
	{
		return -1;
	}

	int value = metacall_value_to_int(ret);

	metacall_value_destroy(ret);

	return value;
}

/* Names of the functions of the script, the script with templates uses the instantiations */
struct script_functions
{
	const char *identity_i32;
	const char *identity_f64;
	const char *pair_i32_f64;
};

static void call_functions(const script_functions &functions, int value)
{
	void *ret = metacall::metacall(functions.identity_i32, value);

	ASSERT_NE((void *)NULL, ret);
	EXPECT_EQ(value, metacall_value_to_int(ret));
	metacall_value_destroy(ret);

	ret = metacall::metacall(functions.identity_f64, value + 0.5);

	ASSERT_NE((void *)NULL, ret);
	EXPECT_EQ(value + 0.5, metacall_value_to_double(ret));
	metacall_value_destroy(ret);

	ret = metacall::metacall(functions.pair_i32_f64, value, value * 2.5);

	ASSERT_NE((void *)NULL, ret);
	EXPECT_EQ(value * 2.5, metacall_value_to_double(ret));
	metacall_value_destroy(ret);
}

/* Libraries compiled by the Rust Loader that are mapped in the process */
static std::set<std::string> libraries_mapped()
{
	std::set<std::string> libraries;

#if defined(__linux__)
	static const std::string deleted(" (deleted)");
	std::ifstream maps("/proc/self/maps");
	std::string line;

	while (std::getline(maps, line))
	{
		size_t position = line.find('/');

		if (position == std::string::npos)
		{
			continue;
		}

		std::string path = line.substr(position);

		/* Memory loads remove their folder when the handle is destroyed */
		if (path.size() > deleted.size() && path.compare(path.size() - deleted.size(), deleted.size(), deleted) == 0)
		{
			path.erase(path.size() - deleted.size());
		}

		if (path.find("/metacall-") != std::string::npos && path.size() > 3 && path.compare(path.size() - 3, 3, ".so") == 0)
		{
			libraries.insert(path);
		}
	}
#endif

	return libraries;
}

/* Thread that calls the script and exits once it is released */
struct thread_gate
{
	std::mutex mutex;
	std::condition_variable cond;
	bool called = false;
	bool released = false;
};

/* Loads the script again after reinitializing metacall several times, the library of each
 * initialization stays loaded because thread local destructors are pending (the main thread
 * calls the script), so each initialization must load a new copy of the script */
static void reinitialize(const char *name, void *data, const script_functions &functions)
{
	static const int cycles = 3;

	const std::string script = script_read(name);

	ASSERT_FALSE(script.empty());

	std::set<std::string> libraries;

	for (int cycle = 1; cycle <= cycles; ++cycle)
	{
		ASSERT_EQ((int)0, (int)metacall_initialize());

		const std::set<std::string> before = libraries_mapped();

		ASSERT_EQ((int)0, (int)metacall_load_from_memory_ex("rs", script.c_str(), script.size(), NULL, data));

		/* The library is a new copy, a library reused from a previous cycle would keep its state */
		EXPECT_EQ((int)0, (int)call_int("calls"));
		EXPECT_EQ((int)0, (int)call_int("destructors"));

		/* Calls from the main thread, the thread local destructor stays pending until the process exits */
		EXPECT_EQ((int)1, (int)call_int("bump"));
		call_functions(functions, cycle);
		EXPECT_EQ((int)4, (int)call_int("calls"));
		EXPECT_EQ((int)4, (int)call_int("thread_calls"));

		/* Calls from a thread that exits before metacall_destroy, its destructor runs while the library is loaded */
		std::thread early([cycle, &functions]() {
			EXPECT_EQ((int)5, (int)call_int("bump"));
			call_functions(functions, cycle);
			EXPECT_EQ((int)4, (int)call_int("thread_calls"));
		});

		early.join();

		EXPECT_EQ((int)8, (int)call_int("calls"));
		EXPECT_EQ((int)1, (int)call_int("destructors"));

		/* Calls from a thread that exits after metacall_destroy, its destructor runs after the loader has been destroyed */
		thread_gate gate;

		std::thread late([cycle, &functions, &gate]() {
			EXPECT_EQ((int)9, (int)call_int("bump"));
			call_functions(functions, cycle);

			std::unique_lock<std::mutex> lock(gate.mutex);

			gate.called = true;
			gate.cond.notify_all();
			gate.cond.wait(lock, [&gate]() { return gate.released; });
		});

		{
			std::unique_lock<std::mutex> lock(gate.mutex);

			gate.cond.wait(lock, [&gate]() { return gate.called; });
		}

		EXPECT_EQ((int)12, (int)call_int("calls"));

		/* Each cycle compiles the script into a library with a different name */
		const std::set<std::string> after = libraries_mapped();

		for (const std::string &library : after)
		{
			if (before.count(library) == 0)
			{
				const std::string library_name = library.substr(library.rfind('/') + 1);

				std::cout << "Cycle " << cycle << " library: " << library_name << std::endl;

				EXPECT_EQ((size_t)0, (size_t)libraries.count(library_name));

				libraries.insert(library_name);
			}
		}

		metacall_destroy();

		{
			std::unique_lock<std::mutex> lock(gate.mutex);

			gate.released = true;
			gate.cond.notify_all();
		}

		/* The destructor of the thread runs here, it must not crash even if the loader has been destroyed */
		late.join();

		const std::set<std::string> destroyed = libraries_mapped();

		for (const std::string &library : after)
		{
			if (before.count(library) == 0)
			{
				std::cout << "Cycle " << cycle << " library after metacall_destroy: " << (destroyed.count(library) > 0 ? "still loaded" : "unloaded") << std::endl;
			}
		}
	}

#if defined(__linux__)
	/* One library per cycle, all of them with a different name */
	EXPECT_EQ((size_t)cycles, (size_t)libraries.size());
#endif
}

/* Scripts without templates are compiled with the standard library linked in, the library
 * stays loaded while a thread local destructor is pending, otherwise it can be unloaded */
TEST_F(metacall_rust_reinitialize_test, NoTemplates)
{
	const script_functions functions = { "identity_i32", "identity_f64", "pair_i32_f64" };

	reinitialize("reinitialize.rs", NULL, functions);
}

/* Scripts with templates share the standard library (so the templates can be instantiated
 * after load), the library is never unloaded because the destructors pin the standard library */
TEST_F(metacall_rust_reinitialize_test, Templates)
{
	template_map data{ { "templates", metacall::array(
										  template_map{ { "identity", metacall::array("i32") } },
										  template_map{ { "identity", metacall::array("f64") } },
										  template_map{ { "pair", metacall::array("i32", "f64") } }) } };

	const script_functions functions = { "identity::<i32>", "identity::<f64>", "pair::<i32, f64>" };

	reinitialize("reinitialize_template.rs", data.to_raw(), functions);
}
