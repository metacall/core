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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

/* Number of JavaScript functions (wrapping a C function) with an N-API finalizer attached */
#define CALLBACK_SIZE 1000

class metacall_node_destroy_finalizer_test : public testing::Test
{
public:
};

static void *c_callback(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	return metacall_value_create_long(0);
}

/*
* This test forces an allocation driven (non-forced) mark-sweep to happen in the last JavaScript
* turn (process 'exit' event), after all the platform tasks have been drained by the event loop.
* The mark-sweep collects the functions that have an N-API finalizer attached, so the second pass
* phantom callbacks of the N-API references are posted as a foreground task that stays pending
* when the environment is freed. In NodeJS 12, the environment cleanup runs as follows:
*
*	1) node::Environment::RunCleanup -> CleanupHandles -> uv_run flushes the pending task, the second pass
*	   callback calls node_napi_env__::CallFinalizer, which defers the finalizer with SetImmediate capturing
*	   the raw napi_env (without taking a reference to it).
*	2) The cleanup hooks run, the napi_env cleanup hook calls Unref and deletes the napi_env.
*	3) The next CleanupHandles -> RunAndClearNativeImmediates runs the deferred finalizer with the deleted
*	   napi_env, producing a use after free (v8::HandleScope::Initialize with env->isolate).
*
* If the task is flushed after step 2 instead, the second pass callback runs over the already deleted
* v8impl::Reference (GlobalHandles::PendingPhantomCallback::Invoke), which is the variant seen in the CI.
*/
TEST_F(metacall_node_destroy_finalizer_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		static const char buffer[] =
			"const v8 = require('v8');\n"
			"let callbacks = [];\n"
			"function register(cb) { callbacks.push(cb); }\n"
			"const new_space = () => v8.getHeapSpaceStatistics().find(s => s.space_name === 'new_space').space_used_size;\n"
			"process.on('exit', () => {\n"
			/* From now on, any GC (even the ones triggered by young generation allocations) is a full non-forced mark-sweep */
			"	v8.setFlagsFromString('--gc-global');\n"
			/* Drop all the functions with N-API finalizers */
			"	callbacks = null;\n"
			/* Allocate young garbage until exactly one GC happens (new space usage drops), then stop allocating,
			* so the second pass callbacks of that mark-sweep remain pending in the platform task queue */
			"	let used = new_space();\n"
			"	for (let i = 0; i < 1000000; ++i) {\n"
			"		new Array(128);\n"
			"		const now = new_space();\n"
			"		if (now < used) break;\n"
			"		used = now;\n"
			"	}\n"
			"});\n"
			"module.exports = { register };\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

		ASSERT_EQ((int)0, (int)metacall_register("c_callback", c_callback, NULL, METACALL_LONG, 0));

		void *c_callback_value = metacall_value_create_function(metacall_function("c_callback"));

		ASSERT_NE((void *)NULL, (void *)c_callback_value);

		for (size_t iterator = 0; iterator < CALLBACK_SIZE; ++iterator)
		{
			void *args[] = { c_callback_value };

			void *ret = metacallv_s("register", args, 1);

			ASSERT_NE((void *)NULL, (void *)ret);

			metacall_value_destroy(ret);
		}

		metacall_value_destroy(c_callback_value);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_destroy();
}
