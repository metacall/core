#include <core_plugin/core_plugin.h>

#include <log/log.h>
#include <metacall/metacall.h>

#include <string.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>

void *load(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	char *tag = metacall_value_to_string(args[0]);
	if (tag == NULL)
	{
		return metacall_value_create_int(1);
	}

	size_t size = metacall_value_count(args[1]);

	char **scripts = (char **)malloc(sizeof(char *) * size);
	void **script_val = metacall_value_to_array(args[1]);
	if (scripts == NULL || script_val == NULL)
	{
		return metacall_value_create_int(1);
	}

	for (size_t i = 0; i < size; ++i)
	{
		scripts[i] = metacall_value_to_string(script_val[i]);
	}

	int ret = metacall_load_from_file(tag, const_cast<const char **>(scripts), size, NULL);

	free(scripts);

	return metacall_value_create_int(ret);
}

void *eval(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;
	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);

	int ret = metacall_load_from_memory(tag, script, strlen(script), NULL);
	return metacall_value_create_int(ret);
}

void *await(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	std::string::size_type idx = func_str.find_first_of('(');
	std::string func_name = func_str.substr(0, idx);

	/* Convert arguments into an array */
	std::string func_args = "[";
	func_args += func_str.substr(idx + 1, func_str.size() - (idx + 2));
	func_args += "]";

	/* Check if function is loaded */
	void *func = NULL;
	if ((func = metacall_function(const_cast<char *>(func_name.c_str()))) == NULL)
	{
		return NULL;
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	std::mutex await_mutex;				/**< Mutex for blocking the process until await is resolved */
	std::condition_variable await_cond; /**< Condition to be fired once await method is resolved or rejected */

	std::unique_lock<std::mutex> lock(await_mutex);

	struct await_data_type
	{
		void *v;
		std::mutex &mutex;
		std::condition_variable &cond;
	};

	struct await_data_type fdata = { NULL, await_mutex, await_cond };

	void *future = metacallfs_await(
		func, func_args.c_str(), func_args.length() + 1, allocator,
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->mutex);
			/* Value must be always copied, it gets deleted after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->cond.notify_one();
			return NULL;
		},
		[](void *result, void *ctx) -> void * {
			struct await_data_type *await_data = static_cast<struct await_data_type *>(ctx);
			std::unique_lock<std::mutex> lock(await_data->mutex);
			/* Value must be always copied, it gets deleted after the scope */
			await_data->v = metacall_value_copy(result);
			await_data->cond.notify_one();
			return NULL;
		},
		static_cast<void *>(&fdata));

	await_cond.wait(lock);

	return fdata.v;
	/* Unused */
	metacall_value_destroy(future);

	metacall_allocator_destroy(allocator);
}

void *call(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	/* Parse function call */
	std::string func_str = metacall_value_to_string(args[0]);

	std::string::size_type idx = func_str.find_first_of('(');
	std::string func_name = func_str.substr(0, idx);

	/* Convert arguments into an array */
	std::string func_args = "[";
	func_args += func_str.substr(idx + 1, func_str.size() - (idx + 2));
	func_args += "]";

	/* Check if function is loaded */
	void *func = NULL;
	if ((func = metacall_function(const_cast<char *>(func_name.c_str()))) == NULL)
	{
		return NULL; //Todo: test this
	}

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	void *result = metacallfs(func, func_args.c_str(), func_args.length() + 1, allocator);

	metacall_allocator_destroy(allocator);
	return result;
}

void *clear(size_t argc, void *args[], void *data)
{
	(void)data;

	if (argc != 2)
	{
		return metacall_value_create_int(1);
	}

	char *tag = metacall_value_to_string(args[0]);
	char *script = metacall_value_to_string(args[1]);

	void *handle = metacall_handle(tag, script);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "handle %s not found in loader (%s)", script, tag);
		return metacall_value_create_int(1);
	}

	if (metacall_clear(handle) != 0)
	{
		return metacall_value_create_int(1);
	}

	return metacall_value_create_int(0);
}

void *inspect(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	metacall_allocator_destroy(allocator);

	return metacall_value_create_string(inspect_str, size);
}

int core_plugin(void *loader, void *handle, void *context)
{
	(void)handle;
	int ret = 0;
	{
		enum metacall_value_id *arg_types = NULL;
		if (metacall_register_loaderv(loader, context, "inspect", inspect, METACALL_STRING, 0, arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: inspect");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "clear", clear, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: clear");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "call", call, METACALL_PTR, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: call");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "await", await, METACALL_PTR, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: await");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "eval", eval, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: eval");
			ret = 1;
		}
	}

	{
		/* Register eval as metacallcli command */
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_STRING };
		if (metacall_register_loaderv(loader, context, "metacallcli-eval", eval, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: metacallcli-eval");
			ret = 1;
		}
	}

	{
		enum metacall_value_id arg_types[] = { METACALL_STRING, METACALL_ARRAY };
		if (metacall_register_loaderv(loader, context, "load", load, METACALL_INT, sizeof(arg_types) / sizeof(arg_types[0]), arg_types) != 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Failed to register function: load");
			ret = 1;
		}
	}

	return ret;
}
