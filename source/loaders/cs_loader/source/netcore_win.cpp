
#include <log/log.h>

#include <cs_loader/netcore_win.h>
#include <cs_loader/host_environment.h>

#include <pal/prebuilt/inc/mscoree.h>
#include <inc/palclr.h>

#include <functional>
#include <memory>

netcore_win::netcore_win(char * dotnet_root, char * dotnet_loader_assembly_path) : netcore(dotnet_root, dotnet_loader_assembly_path), domain_id(0)
{

}


netcore_win::~netcore_win()
{
	this->stop();

	if (this->core_environment != nullptr)
	{
		delete this->core_environment;
	}
}

void netcore_win::stop()
{
	HRESULT hr;

	log_write("metacall", LOG_LEVEL_DEBUG, "Unloading the AppDomain");

	// Wait until unload
	hr = host->UnloadAppDomain((DWORD)this->domain_id, true);

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to unload the AppDomain [ERRORCODE: %d]", hr);
		return;
	}

	// Stop the host
	log_write("metacall", LOG_LEVEL_DEBUG, "Stopping the host");

	hr = this->host->Stop();

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to stop the host [ERRORCODE: %d]", hr);
		return;
	}

	// Release the reference to the host
	log_write("metacall", LOG_LEVEL_DEBUG, "Releasing ICLRRuntimeHost2");

	this->host->Release();
}

bool netcore_win::start()
{
	this->core_environment = new host_environment(this->dotnet_root);

	if (!this->config_assembly_name())
	{
		return false;
	}

	if (!this->create_host())
	{
		return false;
	}

	if (!this->load_main())
	{
		return false;
	}

	if (!this->create_delegates())
	{
		return false;
	}

	return true;
}

bool netcore_win::config_assembly_name()
{
	if (!::GetModuleFileName(NULL, appPath, MAX_LONGPATH))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to get full path: % [ERRORCODE: %d]", this->loader_dll, GetLastError());
		return false;
	}

	if(this->dotnet_loader_assembly_path==NULL)
	{
		wcscpy_s(managedAssemblyFullName, appPath);
	}
	else
	{
		mbstowcs(managedAssemblyFullName, this->dotnet_loader_assembly_path, MAX_LONGPATH);
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading: %s", managedAssemblyFullName);

	wcscpy_s(appNiPath, managedAssemblyFullName);
	wcscat_s(appNiPath, MAX_LONGPATH * 2, W(";"));
	wcscat_s(appNiPath, MAX_LONGPATH * 2, appPath);

	// Construct native search directory paths
	wcscpy_s(nativeDllSearchDirs, appPath);
	wchar_t coreLibraries[MAX_LONGPATH];
	size_t outSize;

	if (_wgetenv_s(&outSize, coreLibraries, MAX_LONGPATH, W("CORE_LIBRARIES")) == 0 && outSize > 0)
	{
		wcscat_s(nativeDllSearchDirs, MAX_LONGPATH * 3, W(";"));
		wcscat_s(nativeDllSearchDirs, MAX_LONGPATH * 3, coreLibraries);
	}
	wcscat_s(nativeDllSearchDirs, MAX_LONGPATH * 3, W(";"));
	wcscat_s(nativeDllSearchDirs, MAX_LONGPATH * 3, this->core_environment->core_clr_directory_path);

	return true;
}

bool netcore_win::create_host()
{
	HRESULT hr;
	bool flagsError = false;

	this->host = this->core_environment->get_clr_runtime_host();

	if (!host)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error in GetCLRRuntimeHost");
		return false;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Setting ICLRRuntimeHost2 startup flags");

	// Default startup flags
	hr = host->SetStartupFlags((STARTUP_FLAGS)
		(STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN |
			STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |
			STARTUP_FLAGS::STARTUP_CONCURRENT_GC));

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to set startup flags [ERRORCODE: %d]", hr);
		flagsError = true;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Starting ICLRRuntimeHost2");

	hr = host->Start();

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to start CoreCLR [ERRORCODE: %d]", hr);
		return false;
	}

	if (flagsError)
	{
		hr = host->GetCurrentAppDomainId(&this->domain_id);

		if (FAILED(hr))
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to GetCurrentAppDomainId [ERRORCODE: %d]", hr);
			return false;
		}
	}
	else
	{
		// Create an AppDomain
		//
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
		const wchar_t * property_keys[] =
		{
			W("TRUSTED_PLATFORM_ASSEMBLIES"),
			W("APP_PATHS"),
			W("APP_NI_PATHS"),
			W("NATIVE_DLL_SEARCH_DIRECTORIES"),
			W("AppDomainCompatSwitch")
		};

		const wchar_t * property_values[] = {
			// TRUSTED_PLATFORM_ASSEMBLIES
			this->core_environment->get_tpa_list(),
			// APP_PATHS
			appPath,
			// APP_NI_PATHS
			appNiPath,
			// NATIVE_DLL_SEARCH_DIRECTORIES
			nativeDllSearchDirs,
			// AppDomainCompatSwitch
			W("UseLatestBehaviorWhenTFMNotSpecified")
		};

		log_write("metacall", LOG_LEVEL_DEBUG, "Creating an AppDomain");

		hr = host->CreateAppDomainWithManager(
			this->core_environment->get_host_exe_name(),   // The friendly name of the AppDomain
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

		if (FAILED(hr))
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed call to CreateAppDomainWithManager [ERRORCODE: %d]", hr);
			return false;
		}
	}

	return true;
}
bool netcore_win::load_main()
{
	HRESULT hr;
	DWORD exitCode = 0;

	hr = this->host->ExecuteAssembly((DWORD)this->domain_id, managedAssemblyFullName, 0, NULL, &exitCode);

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed call to ExecuteAssembly [ERRORCODE: %d]", hr);
		return false;
	}

	return true;
}

bool netcore_win::create_delegate(const wchar_t * delegateName, void ** func)
{
	HRESULT hr;

	hr = this->host->CreateDelegate((DWORD)this->domain_id, this->assembly_name, this->class_name, delegateName, (INT_PTR *)func);

	if (FAILED(hr))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "ailed to CreateDelegate [ERRORCODE: %d]", hr);
		return false;
	}

	return true;
}
