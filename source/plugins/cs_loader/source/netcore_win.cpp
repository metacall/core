#include <cs_loader/netcore_win.h>

#include <stdio.h>
#include <pal/prebuilt/inc/mscoree.h>
#include <inc/palclr.h>

#include <functional>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>
#include <memory>

#include <cs_loader/host_environment.h>

netcore_win::netcore_win()
{
	this->log = new logger();
	this->log->enable();
	this->domain_id = 0;

	*this->log << W("Failed to unload the AppDomain. ERRORCODE: ") << logger::endl;
}


netcore_win::~netcore_win()
{
	this->stop();
	if (this->core_environment != nullptr) {
		delete this->core_environment;
	}

	delete 	this->log;
}

void netcore_win::stop()
{
	HRESULT hr;
	*this->log << W("Unloading the AppDomain") << logger::endl;

	hr = host->UnloadAppDomain(
		(DWORD)this->domain_id,
		true);                          // Wait until done

	if (FAILED(hr)) {
		*this->log << W("Failed to unload the AppDomain. ERRORCODE: ") << hr << logger::endl;
		return;
	}

	//-------------------------------------------------------------

	// Stop the host

	*this->log << W("Stopping the host") << logger::endl;

	hr = this->host->Stop();

	if (FAILED(hr)) {
		*this->log << W("Failed to stop the host. ERRORCODE: ") << hr << logger::endl;
		return;
	}

	//-------------------------------------------------------------

	// Release the reference to the host

	*this->log << W("Releasing ICLRRuntimeHost2") << logger::endl;

	this->host->Release();
}

bool netcore_win::start() {
	this->core_environment = new host_environment(this->log);

	if (!this->config_assembly_name()) {
		return false;
	}

	if (!this->create_host()) {
		return false;
	}
	if (!this->load_main()) {
		return false;
	}
	if (!this->create_delegates()) {
		return false;
	}

	return true;
}

bool netcore_win::config_assembly_name() {

	char* filePart = NULL;
	char loader_dll_char[255];
	wcstombs(loader_dll_char, this->loader_dll, 255);

	if (!::GetFullPathName(loader_dll_char, MAX_LONGPATH, this->appPath, &filePart)) {
		*this->log << W("Failed to get full path: ") << this->loader_dll << logger::endl;
		*this->log << W("Error code: ") << GetLastError() << logger::endl;
		return false;
	}

	strcpy(managedAssemblyFullName, this->appPath);

	*this->log << W("Loading: ") << managedAssemblyFullName << logger::endl;

	strcat(appNiPath, this->appPath);
	strcat(appNiPath, ";");
	strcat(appNiPath, this->appPath);

	// Construct native search directory paths

	strcpy(nativeDllSearchDirs, appPath);
	/*
	char coreLibraries[MAX_LONGPATH];

	size_t outSize;
	if (_wgetenv_s(&outSize, coreLibraries, MAX_LONGPATH, W("CORE_LIBRARIES")) == 0 && outSize > 0)
	{
		strcat(nativeDllSearchDirs, ";");
		strcat(nativeDllSearchDirs, coreLibraries);
	}
	*/
	strcat(nativeDllSearchDirs, ";");
	strcat(nativeDllSearchDirs, this->core_environment->core_clr_directory_path);

	return true;
}

bool netcore_win::create_host() {
	this->host = this->core_environment->get_clr_runtime_host();
	if (!host) {
		*this->log << "fail GetCLRRuntimeHost";
		return false;
	}

	HRESULT hr;

	*this->log << W("Setting ICLRRuntimeHost2 startup flags") << logger::endl;

	// Default startup flags
	hr = host->SetStartupFlags((STARTUP_FLAGS)
		(STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN |
			STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |
			STARTUP_FLAGS::STARTUP_CONCURRENT_GC));
	if (FAILED(hr)) {
		*this->log << W("Failed to set startup flags. ERRORCODE: ") << hr << logger::endl;
		return false;
	}

	*this->log << W("Starting ICLRRuntimeHost2") << logger::endl;

	hr = host->Start();
	if (FAILED(hr)) {
		*this->log << W("Failed to start CoreCLR. ERRORCODE: ") << hr << logger::endl;
		return false;
	}

	//-------------------------------------------------------------

	// Create an AppDomain

	// Allowed property names:
	// APPBASE
	// - The base path of the application from which the exe and other assemblies will be loaded
	//
	// TRUSTED_PLATFORM_ASSEMBLIES
	// - The list of complete paths to each of the fully trusted assemblies
	//
	// APP_PATHS
	// - The list of paths which will be probed by the assembly loader
	//
	// APP_NI_PATHS
	// - The list of additional paths that the assembly loader will probe for ngen images
	//
	// NATIVE_DLL_SEARCH_DIRECTORIES
	// - The list of paths that will be probed for native DLLs called by PInvoke
	//
	const wchar_t *property_keys[] = {
		W("TRUSTED_PLATFORM_ASSEMBLIES"),
		W("APP_PATHS"),
		W("APP_NI_PATHS"),
		W("NATIVE_DLL_SEARCH_DIRECTORIES"),
		W("AppDomainCompatSwitch")
	};
	wchar_t tpa_list_w[0xFFF];
	wchar_t app_path_w[0xFFF];
	wchar_t app_path_ni_w[0xFFF];
	wchar_t nativeDllSearchDirs_w[0xFFF];

	mbtowc(tpa_list_w, this->core_environment->get_tpa_list(), strlen(this->core_environment->get_tpa_list()));
	mbtowc(app_path_w, this->appPath, strlen(this->appPath));
	mbtowc(app_path_ni_w, this->appNiPath, strlen(this->appNiPath));
	mbtowc(nativeDllSearchDirs_w, this->nativeDllSearchDirs, strlen(this->nativeDllSearchDirs));

	const wchar_t *property_values[] = {
		// TRUSTED_PLATFORM_ASSEMBLIES
		tpa_list_w,
		// APP_PATHS
		app_path_w,
		// APP_NI_PATHS
		app_path_ni_w,
		// NATIVE_DLL_SEARCH_DIRECTORIES
		nativeDllSearchDirs_w,
		// AppDomainCompatSwitch
		W("UseLatestBehaviorWhenTFMNotSpecified")
	};


	*this->log << W("Creating an AppDomain") << logger::endl;
	*this->log << W("TRUSTED_PLATFORM_ASSEMBLIES=") << property_values[0] << logger::endl;
	*this->log << W("APP_PATHS=") << property_values[1] << logger::endl;
	*this->log << W("APP_NI_PATHS=") << property_values[2] << logger::endl;
	*this->log << W("NATIVE_DLL_SEARCH_DIRECTORIES=") << property_values[3] << logger::endl;

	wchar_t host_exe_name_w[0xFF];
	mbtowc(host_exe_name_w, this->core_environment->get_host_exe_name(), strlen(this->core_environment->get_host_exe_name()));
	hr = host->CreateAppDomainWithManager(
		host_exe_name_w,   // The friendly name of the AppDomain
											 // Flags:
											 // APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS
											 // - By default CoreCLR only allows platform neutral assembly to be run. To allow
											 //   assemblies marked as platform specific, include this flag
											 //
											 // APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP
											 // - Allows sandboxed applications to make P/Invoke calls and use COM interop
											 //
											 // APPDOMAIN_SECURITY_SANDBOXED
											 // - Enables sandboxing. If not set, the app is considered full trust
											 //
											 // APPDOMAIN_IGNORE_UNHANDLED_EXCEPTION
											 // - Prevents the application from being torn down if a managed exception is unhandled
											 //
		APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS |
		APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP |
		APPDOMAIN_DISABLE_TRANSPARENCY_ENFORCEMENT,
		NULL,                // Name of the assembly that contains the AppDomainManager implementation
		NULL,                    // The AppDomainManager implementation type name
		sizeof(property_keys) / sizeof(wchar_t*),  // The number of properties
		property_keys,
		property_values,
		(DWORD*)&this->domain_id);

	if (FAILED(hr)) {
		*this->log << W("Failed call to CreateAppDomainWithManager. ERRORCODE: ") << hr << logger::endl;
		return false;
	}
	return true;
}
bool netcore_win::load_main() {
	HRESULT hr;
	DWORD exitCode = 0;
	wchar_t full_w[0xFFF];

	mbtowc(full_w, this->managedAssemblyFullName, strlen(this->managedAssemblyFullName));

	hr = this->host->ExecuteAssembly((DWORD)this->domain_id, full_w, 0, NULL, &exitCode);
	if (FAILED(hr)) {
		*this->log << W("Failed call to ExecuteAssembly. ERRORCODE: ") << hr << logger::endl;
		return false;
	}

	return true;
}

bool netcore_win::create_delegate(const wchar_t * delegateName, void** func) {
	HRESULT hr;

	hr = this->host->CreateDelegate((DWORD)this->domain_id, this->assembly_name, this->class_name, delegateName, (INT_PTR*)func);

	if (FAILED(hr)) {
		*this->log << W("Failed to CreateDelegate.") << delegateName << W("ERRORCODE: ") << hr << logger::endl;
		return false;
	}

	return true;
}