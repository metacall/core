/*
 *	Sandbox Plugin by Parra Studios
 *	A plugin implementing sandboxing functionality for MetaCall Core.
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

#include <sandbox_plugin/sandbox_plugin.h>

#include <plugin/plugin_interface.hpp>

#include <seccomp.h>

/* TODO: Use SCMP_ACT_KILL_PROCESS instead of SCMP_ACT_KILL for catching the signal and showing the stack trace? */
/* TODO: We can disable bool (true/false) for string ("allow"/"kill") */
#define SANDBOX_ACTION(value) \
	metacall_value_to_bool(value) == 0L ? SCMP_ACT_KILL : SCMP_ACT_ALLOW

/* Error messages */
#define SANDBOX_INITIALIZE_ERROR "Sandbox plugin failed to initialize a context"
#define SANDBOX_UNAME_ERROR		 "Sandbox plugin failed to set uname syscall permissions"
#define SANDBOX_DESTROY_ERROR	 "Sandbox plugin failed to destroy a context"

void *sandbox_initialize(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_INITIALIZE_ERROR, METACALL_BOOL);

	/* Initialize the seccomp context */
	ctx = seccomp_init(SANDBOX_ACTION(args[0]));

	if (ctx == NULL)
	{
		/* Throw exception if seccomp initialization failed */
		EXTENSION_FUNCTION_THROW(SANDBOX_INITIALIZE_ERROR);
	}

	return metacall_value_create_ptr(ctx);
}

void *sandbox_uname(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_UNAME_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	seccomp_rule_add(ctx, SANDBOX_ACTION(args[1]), SCMP_SYS(uname), 0);
	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_destroy(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_DESTROY_ERROR, METACALL_PTR);

	ctx = metacall_value_to_ptr(args[0]);

	if (ctx == NULL)
	{
		EXTENSION_FUNCTION_THROW(SANDBOX_DESTROY_ERROR ", the first parameter requires a non null pointer");
	}

	seccomp_release(ctx);

	return metacall_value_create_int(0);
}

#if 0 /* TODO: Fork safety */
	#ifdef METACALL_FORK_SAFE
static int sandbox_plugin_post_fork_callback(metacall_pid id, void *data)
{
	(void)id;
	(void)data;

	/* Reset libseccomp library status: https://man7.org/linux/man-pages/man3/seccomp_init.3.html */
	return seccomp_reset(NULL, SANDBOX_DEFAULT_ACTION);
}
	#endif /* METACALL_FORK_SAFE */
#endif

int sandbox_plugin(void *loader, void *handle)
{
	EXTENSION_FUNCTION(METACALL_PTR, sandbox_initialize);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_uname, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_destroy, METACALL_PTR);

#if 0 /* TODO: Fork safety */
	#ifdef METACALL_FORK_SAFE
	/* TODO: This requires to implement multiple callback handling in metacall_fork which is not implemented yet,
	and maybe also passing a context to the pre/post fork callbacks which is not implemented neither */
	metacall_fork(NULL, &sandbox_plugin_post_fork_callback);
	#endif /* METACALL_FORK_SAFE */
#endif

	return 0;
}
