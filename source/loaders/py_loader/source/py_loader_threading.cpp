/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_threading.h>

#include <threading/threading_thread_id.h>

#include <vector>

struct py_thread_state
{
	uint64_t ref_count;
	PyGILState_STATE gstate;

	py_thread_state() :
		ref_count(0) {}

	void ensure()
	{
		if (ref_count == 0)
		{
			gstate = PyGILState_Ensure();
		}

		++ref_count;
	}

	void release()
	{
		if (ref_count > 0)
		{
			--ref_count;

			if (ref_count == 0)
			{
				PyGILState_Release(gstate);
			}
		}
	}
};

static PyThreadState *main_thread_state = NULL;
static uint64_t main_thread_id = 0;
static uint64_t main_thread_ref_count = 0;
thread_local py_thread_state current_thread_state;
thread_local uint64_t current_thread_id = thread_id_get_current();
static std::vector<PyObject *> delayed_destructor;

void py_loader_thread_initialize(const int host)
{
	main_thread_id = thread_id_get_current();

	if (host == 1)
	{
		main_thread_state = PyThreadState_Get();
		main_thread_ref_count++;
	}
}

int py_loader_thread_is_main()
{
	return (int)(main_thread_id == current_thread_id);
}

void py_loader_thread_acquire()
{
	if (main_thread_id == current_thread_id)
	{
		if (main_thread_state != NULL)
		{
			uint64_t ref_count = main_thread_ref_count++;

			if (ref_count == 0)
			{
				PyEval_RestoreThread(main_thread_state);
			}
		}
	}
	else
	{
		current_thread_state.ensure();
	}
}

void py_loader_thread_release()
{
	if (main_thread_id == current_thread_id)
	{
		uint64_t ref_count = main_thread_ref_count;

		if (ref_count > 0)
		{
			ref_count = --main_thread_ref_count;
		}

		if (ref_count == 0)
		{
			main_thread_state = PyEval_SaveThread();
		}
	}
	else
	{
		current_thread_state.release();
	}
}

void py_loader_thread_delayed_destroy(PyObject *obj)
{
	if (main_thread_id == current_thread_id)
	{
		py_loader_thread_acquire();
		Py_DECREF(obj);
		py_loader_thread_release();
	}
	else
	{
		delayed_destructor.push_back(obj);
	}
}

void py_loader_thread_destroy(void)
{
	py_loader_thread_acquire();

	for (auto obj : delayed_destructor)
	{
		Py_DECREF(obj);
	}

	py_loader_thread_release();
}
