#include <cs_loader/simple_netcore.h>

#include <cs_loader/netcore.h>
#if defined(__linux) | defined( linux)
#include <cs_loader/netcore_linux.h>
#else
#include <cs_loader/netcore_win.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>
#include <ostream>

netcore_handle simple_netcore_create(char * dotnet_root) {
#if defined(__linux) | defined( linux)

	netcore_linux * netcore_inp = new netcore_linux(dotnet_root);
#else
	netcore_win *netcore_inp = new netcore_win(dotnet_root);
#endif

	if (netcore_inp->start()) {
		return (netcore_handle)netcore_inp;
	}
	else
	{
		return (netcore_handle)0;
	}
}

reflect_function * simple_netcore_get_functions(netcore_handle handle, int *count) {
	netcore * core = (netcore*)handle;

	return core->get_functions(count);
}

void simple_netcore_destroy(netcore_handle handle) {
	netcore * core = (netcore*)handle;
	delete core;
}

void  simple_netcore_load_script_from_files(netcore_handle handle, char * files[MAX_FILES], size_t size) {
	netcore * core = (netcore*)handle;
	if (core->load_files(files, size)) {

	}
}

void  simple_netcore_load_script_from_assembly(netcore_handle handle, char * file) {
	netcore * core = (netcore*)handle;
	if (core->load_assembly(file)) {

	}
}

void  simple_netcore_load_script_from_memory(netcore_handle handle, const char * buffer, size_t size) {
	netcore * core = (netcore*)handle;

	if (core->load_source((char*)buffer)) {

	}
}
execution_result*  simple_netcore_invoke(netcore_handle handle, const char *func) {
	netcore * core = (netcore*)handle;

	return core->execute((char*)func);
}

execution_result*  simple_netcore_invoke_with_params(netcore_handle handle, const char *func, parameters* params) {
	netcore * core = (netcore*)handle;

	return core->execute_with_params((char*)func, params);
}

void simple_netcore_destroy_execution_result(netcore_handle handle, execution_result* er) {
	netcore * core = (netcore*)handle;

	core->destroy_execution_result(er);
}