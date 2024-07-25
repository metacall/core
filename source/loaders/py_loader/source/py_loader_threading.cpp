/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <Python.h>

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <thread>

struct py_thread_state
{
	uint64_t ref_count;
	PyGILState_STATE gstate;

	py_thread_state() :
		ref_count(1), gstate(PyGILState_Ensure()) {}

	~py_thread_state()
	{
		PyGILState_Release(gstate);
	}
};

static std::map<uint64_t, py_thread_state *> thread_states;
static std::mutex thread_states_mutex;
static PyThreadState *main_thread_state = NULL;
static uint64_t main_thread_id = 0;
static std::atomic_uintmax_t main_thread_ref_count;

void py_loader_thread_initialize()
{
	main_thread_state = PyEval_SaveThread();
	main_thread_id = thread_id_get_current();
	main_thread_ref_count = 0;
}

void py_loader_thread_acquire()
{
	uint64_t current_thread_id = thread_id_get_current();

	if (main_thread_id == current_thread_id)
	{
		if (main_thread_state != NULL)
		{
			uintmax_t ref_count = main_thread_ref_count++;

			if (ref_count == 0)
			{
				PyEval_RestoreThread(main_thread_state);
			}
		}
	}
	else
	{
		thread_states_mutex.lock();
		auto iterator = thread_states.find(current_thread_id);

		if (iterator == thread_states.end())
		{
			thread_states_mutex.unlock();

			py_thread_state *thread_state = new py_thread_state();

			thread_states_mutex.lock();
			thread_states[current_thread_id] = thread_state;
			thread_states_mutex.unlock();
		}
		else
		{
			py_thread_state *thread_state = iterator->second;
			++thread_state->ref_count;
			thread_states_mutex.unlock();
		}
	}
}

void py_loader_thread_release()
{
	uint64_t current_thread_id = thread_id_get_current();

	if (main_thread_id == current_thread_id)
	{
		uint64_t ref_count = main_thread_ref_count.load();

		if (ref_count > 0)
		{
			ref_count = --main_thread_ref_count;

			if (ref_count == 0)
			{
				main_thread_state = PyEval_SaveThread();
			}
		}
	}
	else
	{
		thread_states_mutex.lock();
		auto iterator = thread_states.find(current_thread_id);

		if (iterator == thread_states.end())
		{
			thread_states_mutex.unlock();
		}
		else
		{
			py_thread_state *thread_state = iterator->second;

			if (thread_state->ref_count <= 1)
			{
				thread_states.erase(iterator);
				thread_states_mutex.unlock();

				delete thread_state;
			}
			else
			{
				--thread_state->ref_count;
				thread_states_mutex.unlock();
			}
		}
	}
}