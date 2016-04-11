#include <iostream>
#include <jsapi.h>

static JSClass global_class =
{
	"global", JSCLASS_GLOBAL_FLAGS
};

void error_handler(JSContext *context, const char *message, JSErrorReport *report)
{
	std::cout << "¡¡¡Error!!!" << std::endl;
}

int main(int argc, char *argv[])
{
	JSRuntime *runtime = JS_NewRuntime(8L * 1024L * 1024L);

	if (runtime) {
		JSContext *context = JS_NewContext(runtime, 8192);

		if (context) {
/*			JS_AutoRequest auto_request(context);

			JS::RootedObject global(context, JS_NewGlobalObject(context, &global_class, nullptr, JS::FireOnNewGlobalHook));

			JS::RootedValue ret_val(context);

			if (global && ret_val) {
				const char *sript = "'Hello ' + 'World! It is ' + new Date()";
				const char *filename = "example_script";
				int lineno = 1;

				JS::CompileOptions options(context);

				options.setFileAndLine(filename, lineno);

				bool result = JS::Evaluate(context, global, options, script, strlen(script), &ret_val);

				if (result) {
					JSString *str = ret_val.toString();

					std::cout << "Result: " << JS_EncodeString(context, str) << std::endl;
				}
			}
*/
			JS_DestroyContext(context);
		}

		JS_DestroyRuntime(runtime);
	}

	//JS_ShutDown();

	return 0;
}
