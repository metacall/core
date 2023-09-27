/*
 *	Sandbox Plugin by Parra Studios
 *	A plugin implementing sandboxing functionality for MetaCall Core.
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

#include <sandbox_plugin/sandbox_plugin.h>

#include <seccomp.h>

#include <preprocessor/preprocessor.h>

#include <log/log.h>

#include <metacall/metacall.h>

/* TODO: Use SCMP_ACT_KILL_PROCESS for catching the signal and showing the stack trace? */
#define SANDBOX_DEFAULT_ACTION SCMP_ACT_ALLOW // SCMP_ACT_KILL

void *sandbox_initialize(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	(void)argc;
	(void)args;
	(void)data;

	/* Initialize the scmp context */
	ctx = seccomp_init(SANDBOX_DEFAULT_ACTION);

	if (ctx == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to initialize a context.");
	}

	return metacall_value_create_ptr(ctx);
}

/*int sandbox_uname(void *ctx, int allow)
{
    seccomp_rule_add(ctx, allow == 0L ? SCMP_ACT_KILL : SCMP_ACT_ALLOW, SCMP_SYS(uname), 0);
    seccomp_load(ctx);
	return 0;
}*/

void *sandbox_uname(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	(void)data;

	if (argc != 2)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to set uname syscall permissions. The required number of argumens is one, received: %" PRIuS, argc);
		goto error_args;
	}

	if (metacall_value_id(args[0]) != METACALL_PTR)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to set uname syscall permissions. "
											   "The first parameter requires a pointer to the context, received: %s",
			metacall_value_type_name(args[0]));
		goto error_args;
	}

	if (metacall_value_id(args[1]) != METACALL_BOOL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to set uname syscall permissions. "
											   "The second parameter requires a boolean indicating if it is allowed or not, received: %s",
			metacall_value_type_name(args[1]));
		goto error_args;
	}

	ctx = metacall_value_to_ptr(args[0]);

	seccomp_rule_add(ctx, metacall_value_to_bool(args[1]) == 0L ? SCMP_ACT_KILL : SCMP_ACT_ALLOW, SCMP_SYS(uname), 0);
	seccomp_load(ctx);

	return metacall_value_create_int(0);

error_args:
	return metacall_value_create_int(1);
}

void *sandbox_destroy(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	(void)data;

	if (argc != 1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to destroy a context. The required number of argumens is one, received: %" PRIuS, argc);
		goto error_args;
	}

	if (metacall_value_id(args[0]) != METACALL_PTR)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to destroy a context. "
											   "The first parameter requires a pointer to the context, received: %s",
			metacall_value_type_name(args[0]));
		goto error_args;
	}

	ctx = metacall_value_to_ptr(args[0]);

	if (ctx == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Sandbox plugin failed to destroy a context. "
											   "The first parameter requires a non null pointer");
		goto error_args;
	}

	seccomp_release(ctx);

	return metacall_value_create_int(0);

error_args:
	return metacall_value_create_int(1);
}

#ifdef METACALL_FORK_SAFE
static int sandbox_plugin_post_fork_callback(metacall_pid id, void *data)
{
	(void)id;
	(void)data;

	/* Reset libseccomp library status: https://man7.org/linux/man-pages/man3/seccomp_init.3.html */
	return seccomp_reset(NULL, SANDBOX_DEFAULT_ACTION);
}
#endif /* METACALL_FORK_SAFE */

int sandbox_plugin(void *loader, void *handle, void *context)
{
	(void)handle;

#define SANDBOX_FUNCTION_IMPL_0(ret, name)                                                                        \
	do                                                                                                            \
	{                                                                                                             \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, 0, NULL) != 0)    \
		{                                                                                                         \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name)); \
			return 1;                                                                                             \
		}                                                                                                         \
	} while (0)

#define SANDBOX_FUNCTION_IMPL(ret, name, ...)                                                                                                          \
	do                                                                                                                                                 \
	{                                                                                                                                                  \
		enum metacall_value_id arg_types[] = { __VA_ARGS__ };                                                                                          \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, PREPROCESSOR_ARGS_COUNT(__VA_ARGS__), arg_types) != 0) \
		{                                                                                                                                              \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name));                                      \
			return 1;                                                                                                                                  \
		}                                                                                                                                              \
	} while (0)

#define SANDBOX_FUNCTION(ret, name, ...)                                   \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__),                  \
		SANDBOX_FUNCTION_IMPL_0(ret, PREPROCESSOR_CONCAT(sandbox_, name)), \
		SANDBOX_FUNCTION_IMPL(ret, PREPROCESSOR_CONCAT(sandbox_, name), __VA_ARGS__))

	SANDBOX_FUNCTION(METACALL_PTR, initialize);
	SANDBOX_FUNCTION_IMPL(METACALL_INT, sandbox_uname, METACALL_PTR, METACALL_BOOL);
	SANDBOX_FUNCTION_IMPL(METACALL_INT, sandbox_destroy, METACALL_PTR);

#undef SANDBOX_FUNCTION_IMPL_0
#undef SANDBOX_FUNCTION_IMPL
#undef SANDBOX_FUNCTION

#if 0 /* TODO: Fork safety */
	#ifdef METACALL_FORK_SAFE
	/* TODO: This requires to implement multiple callback handling in metacall_fork which is not implemented yet,
	and maybe also passing a context to the pre/post fork callbacks which is not implemented neither */
	metacall_fork(NULL, &sandbox_plugin_post_fork_callback);
	#endif /* METACALL_FORK_SAFE */
#endif

	return 0;
}
