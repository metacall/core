/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <benchmark/benchmark.h>

#include <Python.h>

class metacall_py_c_api_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &state)
	{
		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def int_mem_type(left: int, right: int) -> int:\n"
			"\treturn 0;";

		static const char name[] = "int_mem_type";

		if (Py_IsInitialized() == 0)
		{
			Py_InitializeEx(0);
		}

#if !(PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9)
		if (PyEval_ThreadsInitialized() == 0)
		{
			PyEval_InitThreads();
		}
#endif

		compiled = Py_CompileString(buffer, name, Py_file_input);

		instance = PyImport_ExecCodeModule(name, compiled);

		dict = PyModule_GetDict(instance);

		Py_INCREF(dict);

		func = PyDict_GetItemString(dict, "int_mem_type");

		Py_INCREF(func);

		if (!PyCallable_Check(func))
		{
			state.SkipWithError("An error ocurred during script loading");
		}
	}

	void TearDown(benchmark::State &state)
	{
		Py_DECREF(func);
		Py_DECREF(dict);
		Py_DECREF(instance);
		Py_DECREF(compiled);

		if (Py_IsInitialized() != 0)
		{
			if (PyErr_Occurred() != NULL)
			{
				state.SkipWithError("An error ocurred during the execution");
			}

			Py_Finalize();
		}
	}

protected:
	PyObject *compiled;
	PyObject *instance;
	PyObject *dict;
	PyObject *func;
};

BENCHMARK_DEFINE_F(metacall_py_c_api_bench, call_object)
(benchmark::State &state)
{
	const int64_t call_count = 1000000;
	const int64_t call_size = sizeof(long) * 3; // (long, long) -> long

	for (auto _ : state)
	{
		state.PauseTiming();

		PyObject *tuple_args = PyTuple_New(2);

		PyObject *args[2] = {
			PyLong_FromLong(0L),
			PyLong_FromLong(0L)
		};

		PyTuple_SetItem(tuple_args, 0, args[0]);
		PyTuple_SetItem(tuple_args, 1, args[1]);

		state.ResumeTiming();

		for (int64_t it = 0; it < call_count; ++it)
		{
			PyObject *ret = PyObject_CallObject(func, tuple_args);

			state.PauseTiming();

			if (ret == NULL)
			{
				state.SkipWithError("Null return value from int_mem_type");
			}

			if (PyLong_AsLong(ret) != 0L)
			{
				state.SkipWithError("Invalid return value from int_mem_type");
			}

			Py_DECREF(ret);

			state.ResumeTiming();
		}

		state.PauseTiming();

		Py_DECREF(tuple_args);

		state.ResumeTiming();
	}

	state.SetLabel("MetaCall Python C API Benchmark - Call Object");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_py_c_api_bench, call_object)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

BENCHMARK_MAIN();
