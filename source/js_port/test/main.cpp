/*
 *	MetaCall SWIG Wrapper by Parra Studios
 *	A complete infraestructure for supporting multiple langauge bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 5.1.117 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <map>

/* -- Definitions -- */

#if defined(_WIN32)
#	define JS_PORT_TEST_WIN 1
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)
#	define JS_PORT_TEST_UNIX 1
#else
#	error "Unsuported dynamic library link type"
#endif

/* -- Headers -- */

#if defined(JS_PORT_TEST_WIN)
#	include <windows.h>
#elif defined(JS_PORT_TEST_UNIX)
#	include <dlfcn.h>
#endif

/* -- Namespaces -- */

using namespace v8;

/* -- Type Definitions -- */

#if (NODE_MODULE_VERSION < 0x000C)
	typedef void (*module_initialize)(Handle<Object>);
#else
	typedef void (*module_initialize)(Handle<Object>, Handle<Object>);
#endif

#if defined(JS_PORT_TEST_WIN)
	typedef FARPROC module_handle_symbol;
	typedef HMODULE module_handle;
#elif defined(JS_PORT_TEST_UNIX)
	typedef void * module_handle_symbol;
	typedef void * module_handle;
#endif

typedef std::map<std::string, module_handle> module_map;

/* -- Methods -- */

Local<Context> CreateShellContext(Isolate* isolate);
void RunShell(Local<Context> context, Platform* platform);
int RunMain(Isolate* isolate, Platform* platform, int argc, char * argv[]);
bool ExecuteString(Isolate* isolate, Local<String> source, Local<Value> name, bool print_result, bool report_exceptions);
void Print(const FunctionCallbackInfo<Value>& args);
void Read(const FunctionCallbackInfo<Value>& args);
int StrEndsWith(const char * const str, const char * const suffix);
void ModulesClear(void);
void Load(const FunctionCallbackInfo<Value>& args);
void Quit(const FunctionCallbackInfo<Value>& args);
void Version(const FunctionCallbackInfo<Value>& args);
void AssertEq(const FunctionCallbackInfo<Value>& args);
void AssertNe(const FunctionCallbackInfo<Value>& args);
void SetEnv(const FunctionCallbackInfo<Value>& args);
MaybeLocal<String> ReadFile(Isolate* isolate, const char * name);
void ReportException(Isolate* isolate, TryCatch* handler);

/* -- Member Data -- */

static module_map modules;
static bool run_shell;

/* -- Classes -- */

class ShellArrayBufferAllocator : public ArrayBuffer::Allocator
{
  public:
	virtual void * Allocate(size_t length)
	{
		void * data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void * AllocateUninitialized(size_t length)
	{
		return malloc(length);
	}
	virtual void Free(void * data, size_t)
	{
		free(data);
	}
};

/* -- Entry Point -- */

int main(int argc, char * argv[])
{
	V8::InitializeICU();

	V8::InitializeExternalStartupData(argv[0]);

	Platform * platform = platform::CreateDefaultPlatform();

	V8::InitializePlatform(platform);

	V8::Initialize();

	V8::SetFlagsFromCommandLine(&argc, argv, true);

	ShellArrayBufferAllocator array_buffer_allocator;

	Isolate::CreateParams create_params;

	create_params.array_buffer_allocator = &array_buffer_allocator;

	Isolate * isolate = Isolate::New(create_params);

	run_shell = (argc == 1);

	int result;
	{
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_scope(isolate);
		Local<Context> context = CreateShellContext(isolate);

		if (context.IsEmpty())
		{
			fprintf(stderr, "Error creating context\n");
			return 1;
		}

		Context::Scope context_scope(context);

		result = RunMain(isolate, platform, argc, argv);

		if (run_shell)
		{
			RunShell(context, platform);
		}
	}

	ModulesClear();

	isolate->Dispose();

	V8::Dispose();

	V8::ShutdownPlatform();

	delete platform;

	return result;
}

/* -- Methods -- */

// Extracts a C string from a V8 Utf8Value.
const char * ToCString(const String::Utf8Value& value)
{
	return *value ? *value : "<string conversion failed>";
}

// Creates a new execution environment containing the built-in
// functions.
Local<Context> CreateShellContext(Isolate* isolate)
{
	// Create a template for the global object.
	Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
	// Bind the global 'print' function to the C++ Print callback.
	global->Set(
		String::NewFromUtf8(isolate, "print", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, Print));
	// Bind the global 'read' function to the C++ Read callback.
	global->Set(String::NewFromUtf8(
		isolate, "read", NewStringType::kNormal).ToLocalChecked(),
		FunctionTemplate::New(isolate, Read));
	// Bind the global 'load' function to the C++ Load callback.
	global->Set(String::NewFromUtf8(
		isolate, "load", NewStringType::kNormal).ToLocalChecked(),
		FunctionTemplate::New(isolate, Load));
	// Bind the 'quit' function
	global->Set(String::NewFromUtf8(
		isolate, "quit", NewStringType::kNormal).ToLocalChecked(),
		FunctionTemplate::New(isolate, Quit));
	// Bind the 'version' function
	global->Set(
		String::NewFromUtf8(isolate, "version", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, Version));
	// Bind the global 'assert_eq' function to the C++ AssertEq callback.
	global->Set(
		String::NewFromUtf8(isolate, "assert_eq", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, AssertEq));
	// Bind the global 'assert_ne' function to the C++ AssertNe callback.
	global->Set(
		String::NewFromUtf8(isolate, "assert_ne", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, AssertNe));
	// Bind the global 'setenv' function to the C++ SetEnv callback.
	global->Set(
		String::NewFromUtf8(isolate, "setenv", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, SetEnv));

	return Context::New(isolate, NULL, global);
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.	Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const FunctionCallbackInfo<Value>& args)
{
	bool first = true;
	for (int i = 0; i < args.Length(); i++)
	{
		HandleScope handle_scope(args.GetIsolate());
		if (first)
		{
			first = false;
		}
		else
		{
			printf(" ");
		}
		String::Utf8Value str(args[i]);
		const char * cstr = ToCString(str);
		printf("%s", cstr);
	}
	printf("\n");
	fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.	This function loads the content of the file named in
// the argument into a JavaScript string.
void Read(const FunctionCallbackInfo<Value>& args)
{
	if (args.Length() != 1)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Bad parameters",
			NewStringType::kNormal).ToLocalChecked());
		return;
	}
	String::Utf8Value file(args[0]);
	if (*file == NULL)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Error loading file",
			NewStringType::kNormal).ToLocalChecked());
		return;
	}
	Local<String> source;
	if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source))
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Error loading file",
			NewStringType::kNormal).ToLocalChecked());
		return;
	}
	args.GetReturnValue().Set(source);
}

int StrEndsWith(const char * const str, const char * const suffix)
{
	if (str == NULL || suffix == NULL)
	{
		return 1;
	}

	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);

	if (str_len < suffix_len)
		return 1;

	return strncmp(&str[str_len - suffix_len], suffix, suffix_len);
}

void ModulesClear()
{
	for (module_map::iterator it = modules.begin();
			it != modules.end(); ++it)
	{
		#if defined(JS_PORT_TEST_WIN)
			FreeLibrary(it->second);
		#elif defined(JS_PORT_TEST_UNIX)
			dlclose(it->second);
		#endif
	}
}

module_handle LoadLibraryImpl(const char * lib_str)
{
	#if defined(JS_PORT_TEST_WIN)
		std::string lib_name(lib_str);

		lib_name += ".dll";

		return LoadLibrary(lib_name.c_str());

	#elif defined(JS_PORT_TEST_UNIX)
		std::string lib_name(lib_str);

		lib_name += ".so";

		module_handle lib = dlopen(lib_name.c_str(), RTLD_NOW | RTLD_GLOBAL);

		if (lib == NULL)
		{
			lib_name = "lib" + lib_name;

			lib = dlopen(lib_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
		}

		return lib;
	#endif

	return NULL;
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.	Loads, compiles and executes its argument
// JavaScript file.
void Load(const FunctionCallbackInfo<Value>& args)
{
	for (int i = 0; i < args.Length(); i++)
	{
		HandleScope handle_scope(args.GetIsolate());

		String::Utf8Value file(args[i]);

		if (*file == NULL)
		{
			args.GetIsolate()->ThrowException(
				String::NewFromUtf8(args.GetIsolate(), "Error loading file",
				NewStringType::kNormal).ToLocalChecked());
			return;
		}

		const char * file_str = ToCString(file);

		if (StrEndsWith(file_str, ".js") != 0)
		{
			module_handle lib = LoadLibraryImpl(file_str);

			if (lib == NULL)
			{
				std::string err_str = "Error loading library";

				#if defined(JS_PORT_TEST_WIN)
					/* TODO */
				#elif defined(JS_PORT_TEST_UNIX)
					err_str += " (";

					err_str += dlerror();

					err_str += ")";
				#endif

				args.GetIsolate()->ThrowException(
					String::NewFromUtf8(args.GetIsolate(), err_str.c_str(),
					NewStringType::kNormal).ToLocalChecked());

				return;
			}

			static module_handle_symbol module_initialize_addr = NULL;

			static const char symbol_suffix[] = "_initialize";

			std::string symbol_str(file_str);

			symbol_str += symbol_suffix;

			module_initialize_addr =
				#if defined(JS_PORT_TEST_WIN)
					GetProcAddress(lib, symbol_str.c_str());
				#elif defined(JS_PORT_TEST_UNIX)
					dlsym(lib, symbol_str.c_str());
				#endif


			if (module_initialize_addr == NULL)
			{
				std::string err_str = "Error loading library, not entry point found (";

				err_str += symbol_str + ")";

				args.GetIsolate()->ThrowException(
					String::NewFromUtf8(args.GetIsolate(), err_str.c_str(),
					NewStringType::kNormal).ToLocalChecked());
				return;
			}

			module_initialize module_init = (module_initialize)module_initialize_addr;

			printf("Loading entry point: %s (%p)\n", symbol_str.c_str(), module_init);

			/* MetaCall JS Port Bindings */
			Local<Context> context(args.GetIsolate()->GetCurrentContext());
			Local<Object> global(context->Global());

			#if (NODE_MODULE_VERSION < 0x000C)
				module_init(global);
			#else
				module_init(global, NULL);
			#endif

			modules[file_str] = lib;
		}
		else
		{
			Local<String> source;

			if (!ReadFile(args.GetIsolate(), file_str).ToLocal(&source))
			{
				args.GetIsolate()->ThrowException(
					String::NewFromUtf8(args.GetIsolate(), "Error loading file",
					NewStringType::kNormal).ToLocalChecked());
				return;
			}

			if (!ExecuteString(args.GetIsolate(), source, args[i], false, false))
			{
				args.GetIsolate()->ThrowException(
					String::NewFromUtf8(args.GetIsolate(), "Error executing file",
					NewStringType::kNormal).ToLocalChecked());
				return;
			}
		}
	}
}

// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.	Quits.
void Quit(const FunctionCallbackInfo<Value>& args)
{
	// If not arguments are given args[0] will yield undefined which
	// converts to the integer value 0.
	int exit_code =
		args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);

	ModulesClear();

	fflush(stdout);
	fflush(stderr);
	exit(exit_code);
}

void Version(const FunctionCallbackInfo<Value>& args)
{
	args.GetReturnValue().Set(
		String::NewFromUtf8(args.GetIsolate(), V8::GetVersion(),
		NewStringType::kNormal).ToLocalChecked());
}

void AssertEq(const FunctionCallbackInfo<Value>& args)
{
	const bool result = args[0]->Equals(args[1]);

	if (!result)
	{
		String::Utf8Value left(args[0]);
		String::Utf8Value right(args[1]);

		std::string assert_msg("Assertion Equal failed (");

		assert_msg += ToCString(left);

		assert_msg += " != ";

		assert_msg += ToCString(right);

		assert_msg += ")";

		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), assert_msg.c_str(),
			NewStringType::kNormal).ToLocalChecked());
	}

	args.GetReturnValue().Set(result);
}

void AssertNe(const FunctionCallbackInfo<Value>& args)
{
	const bool result = !args[0]->Equals(args[1]);

	if (!result)
	{
		String::Utf8Value left(args[0]);
		String::Utf8Value right(args[1]);

		std::string assert_msg("Assertion Not Equal failed (");

		assert_msg += ToCString(left);

		assert_msg += " == ";

		assert_msg += ToCString(right);

		assert_msg += ")";

		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), assert_msg.c_str(),
			NewStringType::kNormal).ToLocalChecked());
	}

	args.GetReturnValue().Set(result);
}

void SetEnv(const FunctionCallbackInfo<Value>& args)
{
	if (args.Length() != 2)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "setenv() invalid number of arguments",
			NewStringType::kNormal).ToLocalChecked());

		return;
	}

	String::Utf8Value env_str(args[0]);
	String::Utf8Value value_str(args[1]);

	if (*env_str == NULL)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "setenv() invalid environment variable name",
			NewStringType::kNormal).ToLocalChecked());

		return;
	}

	if (*value_str == NULL)
	{
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "setenv() invalid environment variable value",
			NewStringType::kNormal).ToLocalChecked());

		return;
	}

	#ifdef V8_OS_WIN
		_putenv_s(*env_str, *value_str);
	#else
		setenv(*env_str, *value_str, 1);
	#endif
}

// Reads a file into a v8 string.
MaybeLocal<String> ReadFile(Isolate* isolate, const char * name)
{
	FILE* file = fopen(name, "rb");
	if (file == NULL) return MaybeLocal<String>();

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	char * chars = new char[size + 1];
	chars[size] = '\0';
	for (size_t i = 0; i < size;)
	{
		i += fread(&chars[i], 1, size - i, file);
		if (ferror(file))
		{
			fclose(file);
			return MaybeLocal<String>();
		}
	}
	fclose(file);

	if (chars[0] == '#' && chars[1] == '!')
	{
		chars[0] = '/';
		chars[1] = '/';
	}

	MaybeLocal<String> result = String::NewFromUtf8(
		isolate, chars, NewStringType::kNormal, static_cast<int>(size));
	delete[] chars;
	return result;
}

// Process remaining command line arguments and execute files
int RunMain(Isolate* isolate, Platform* platform, int argc,
	char * argv[])
{
	for (int i = 1; i < argc; i++)
	{
		const char * str = argv[i];
		if (strcmp(str, "--shell") == 0)
		{
			run_shell = true;
		}
		else if (strcmp(str, "-f") == 0)
		{
			// Ignore any -f flags for compatibility with the other stand-
			// alone JavaScript engines.
			continue;
		}
		else if (strncmp(str, "--", 2) == 0)
		{
			fprintf(stderr,
				"Warning: unknown flag %s.\nTry --help for options\n", str);
		}
		else if (strcmp(str, "-e") == 0 && i + 1 < argc)
		{
			// Execute argument given to -e option directly.
			Local<String> file_name =
				String::NewFromUtf8(isolate, "unnamed",
				NewStringType::kNormal).ToLocalChecked();
			Local<String> source;
			if (!String::NewFromUtf8(isolate, argv[++i],
				NewStringType::kNormal)
				.ToLocal(&source))
			{
				return 1;
			}
			bool success = ExecuteString(isolate, source, file_name, false, true);
			while (platform::PumpMessageLoop(platform, isolate)) continue;
			if (!success) return 1;
		}
		else
		{
			// Use all other arguments as names of files to load and run.
			Local<String> file_name =
				String::NewFromUtf8(isolate, str, NewStringType::kNormal)
				.ToLocalChecked();
			Local<String> source;
			if (!ReadFile(isolate, str).ToLocal(&source))
			{
				fprintf(stderr, "Error reading '%s'\n", str);
				continue;
			}
			bool success = ExecuteString(isolate, source, file_name, false, true);
			while (platform::PumpMessageLoop(platform, isolate)) continue;
			if (!success) return 1;
		}
	}
	return 0;
}

// The read-eval-execute loop of the shell.
void RunShell(Local<Context> context, Platform* platform)
{
	fprintf(stderr, "V8 version %s [sample shell]\n", V8::GetVersion());
	static const int kBufferSize = 256;
	// Enter the execution environment before evaluating any code.
	Context::Scope context_scope(context);
	Local<String> name(
		String::NewFromUtf8(context->GetIsolate(), "(shell)",
		NewStringType::kNormal).ToLocalChecked());
	while (true)
	{
		char buffer[kBufferSize];
		fprintf(stderr, "> ");
		char * str = fgets(buffer, kBufferSize, stdin);
		if (str == NULL) break;
		HandleScope handle_scope(context->GetIsolate());
		ExecuteString(
			context->GetIsolate(),
			String::NewFromUtf8(context->GetIsolate(), str,
			NewStringType::kNormal).ToLocalChecked(),
			name, true, true);
		while (platform::PumpMessageLoop(platform, context->GetIsolate()))
			continue;
	}
	fprintf(stderr, "\n");
}

// Executes a string within the current v8 context.
bool ExecuteString(Isolate* isolate, Local<String> source,
	Local<Value> name, bool print_result,
	bool report_exceptions)
{
	HandleScope handle_scope(isolate);
	TryCatch try_catch(isolate);
	ScriptOrigin origin(name);
	Local<Context> context(isolate->GetCurrentContext());
	Local<Script> script;
	if (!Script::Compile(context, source, &origin).ToLocal(&script))
	{
		// Print errors that happened during compilation.
		if (report_exceptions)
			ReportException(isolate, &try_catch);
		return false;
	}
	else
	{
		Local<Value> result;
		if (!script->Run(context).ToLocal(&result))
		{
			assert(try_catch.HasCaught());
			// Print errors that happened during execution.
			if (report_exceptions)
				ReportException(isolate, &try_catch);
			return false;
		}
		else
		{
			assert(!try_catch.HasCaught());
			if (print_result && !result->IsUndefined())
			{
				// If all went well and the result wasn't undefined then print
				// the returned value.
				String::Utf8Value str(result);
				const char * cstr = ToCString(str);
				printf("%s\n", cstr);
			}
			return true;
		}
	}
}

void ReportException(Isolate* isolate, TryCatch* try_catch)
{
	HandleScope handle_scope(isolate);

	String::Utf8Value exception(try_catch->Exception());

	const char * exception_string = ToCString(exception);

	Local<Message> message = try_catch->Message();

	if (message.IsEmpty())
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	}

	// Print (filename):(line number): (message).
	String::Utf8Value filename(message->GetScriptOrigin().ResourceName());

	Local<Context> context(isolate->GetCurrentContext());

	const char * filename_string = ToCString(filename);

	int linenum = message->GetLineNumber(context).FromJust();

	fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);

	// Print line of source code.
	String::Utf8Value sourceline(
		message->GetSourceLine(context).ToLocalChecked());

	const char * sourceline_string = ToCString(sourceline);

	fprintf(stderr, "%s\n", sourceline_string);

	// Print wavy underline (GetUnderline is deprecated).
	int start = message->GetStartColumn(context).FromJust();

	for (int i = 0; i < start; i++)
	{
		fprintf(stderr, " ");
	}

	int end = message->GetEndColumn(context).FromJust();

	for (int i = start; i < end; i++)
	{
		fprintf(stderr, "^");
	}

	fprintf(stderr, "\n");

	Local<Value> stack_trace_string;

	if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
		stack_trace_string->IsString() &&
		Local<String>::Cast(stack_trace_string)->Length() > 0)
	{
		String::Utf8Value stack_trace(stack_trace_string);

		const char * stack_trace_string = ToCString(stack_trace);

		fprintf(stderr, "%s\n", stack_trace_string);
	}
}
