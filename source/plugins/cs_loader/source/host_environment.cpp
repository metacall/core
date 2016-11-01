#include <cs_loader/host_environment.h>


host_environment::host_environment(logger *logger) : log(logger), clr_runtime_host(nullptr)
{
	// Discover the path to this exe's module. All other files are expected to be in the same directory.
	DWORD thisModuleLength = ::GetModuleFileName(::GetModuleHandleW(nullptr), this->host_path, MAX_LONGPATH);

	// Search for the last backslash in the host path.
	int lastBackslashIndex;
	for (lastBackslashIndex = thisModuleLength - 1; lastBackslashIndex >= 0; lastBackslashIndex--) {
		if (this->host_path[lastBackslashIndex] == '\\') {
			break;
		}
	}

	// Copy the directory path
	::strncpy(this->host_directory_path, this->host_path, lastBackslashIndex + 1);

	// Save the exe name
	this->host_exe_name = this->host_path + lastBackslashIndex + 1;

	*this->log << W("Host directory: ") << this->host_directory_path << logger::endl;

	// Check for %CORE_ROOT% and try to load CoreCLR.dll from it if it is set
	char * coreRootTmp;
	char  coreRoot[MAX_LONGPATH];
	this->core_clr_module = NULL; // Initialize this here since we don't call TryLoadCoreCLR if CORE_ROOT is unset.
	coreRootTmp = getenv("CORE_ROOT");
	if (coreRootTmp)
	{
		strcpy(coreRoot, coreRootTmp);
		strcat(coreRoot, "\\");
		this->core_clr_module = this->try_load_core_clr(coreRoot);
	}
	else
	{
		*this->log << W("CORE_ROOT not set; skipping") << logger::endl;
		*this->log << W("You can set the environment variable CORE_ROOT to point to the path") << logger::endl;
		*this->log << W("where CoreCLR.dll lives to help this executable find it.") << logger::endl;
	}

	// Try to load CoreCLR from the directory that this exexutable is in
	if (!this->core_clr_module) {
		this->core_clr_module = this->try_load_core_clr(this->host_directory_path);
	}

	if (this->core_clr_module) {

		// Save the directory that CoreCLR was found in
		DWORD modulePathLength = ::GetModuleFileName(this->core_clr_module, this->core_clr_directory_path, MAX_LONGPATH);

		// Search for the last backslash and terminate it there to keep just the directory path with trailing slash
		for (lastBackslashIndex = modulePathLength - 1; lastBackslashIndex >= 0; lastBackslashIndex--) {
			if (this->core_clr_directory_path[lastBackslashIndex] == '\\') {
				this->core_clr_directory_path[lastBackslashIndex + 1] = '\0';
				break;
			}
		}

	}
	else {
		*this->log << W("Unable to load ") << core_clr_dll << logger::endl;
	}
}


host_environment::host_environment()
{
}


host_environment::~host_environment()
{
	if (this->core_clr_module) {
		// Free the module. This is done for completeness, but in fact CoreCLR.dll 
		// was pinned earlier so this call won't actually free it. The pinning is
		// done because CoreCLR does not support unloading.
		::FreeLibrary(this->core_clr_module);
	}
}

// Attempts to load CoreCLR.dll from the given directory.
// On success pins the dll, sets m_coreCLRDirectoryPath and returns the HMODULE.
// On failure returns nullptr.
HMODULE host_environment::try_load_core_clr(const char* directory_path) {

	char coreCLRPath[MAX_LONGPATH]="";
	strcpy(coreCLRPath, directory_path);
	strcat(coreCLRPath, core_clr_dll);

	*this->log << W("Attempting to load: ") << coreCLRPath << logger::endl;

	HMODULE result = ::LoadLibraryEx(coreCLRPath, NULL, 0);
	if (!result) {
		*this->log << W("Failed to load: ") << coreCLRPath << logger::endl;
		*this->log << W("Error code: ") << GetLastError() << logger::endl;
		return nullptr;
	}

	// Pin the module - CoreCLR.dll does not support being unloaded.
	HMODULE dummy_coreCLRModule;
	if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, coreCLRPath, &dummy_coreCLRModule)) {
		*this->log << W("Failed to pin: ") << coreCLRPath << logger::endl;
		return nullptr;
	}

	char coreCLRLoadedPath[MAX_LONGPATH];
	::GetModuleFileName(result, coreCLRLoadedPath, MAX_LONGPATH);

	*this->log << W("Loaded: ") << coreCLRLoadedPath << logger::endl;

	return result;
}

bool host_environment::tpa_list_contains_file(_In_z_ char* fileNameWithoutExtension, _In_reads_(countExtensions) char** rgTPAExtensions, int countExtensions) {

	if (!this->tpa_list.c_str()) return false;

	for (int iExtension = 0; iExtension < countExtensions; iExtension++)
	{
		char fileName[MAX_LONGPATH];
		strcat(fileName, "\\"); // So that we don't match other files that end with the current file name
		strcat(fileName, fileNameWithoutExtension);
		strcat(fileName, rgTPAExtensions[iExtension] + 1);
		strcat(fileName, ";"); // So that we don't match other files that begin with the current file name

		if (strstr(this->tpa_list.c_str(), fileName))
		{
			return true;
		}
	}
	return false;
}

void host_environment::remove_extension_and_ni(_In_z_ char* fileName)
{
	// Remove extension, if it exists
	char* extension = strrchr(fileName, '.');

	if (extension != NULL)
	{
		extension[0] = '\0';

		// Check for .ni
		size_t len = strlen(fileName);
		if (len > 3 &&
			fileName[len - 1] == 'i' &&
			fileName[len - 2] == 'n' &&
			fileName[len - 3] == '.')
		{
			fileName[len - 3] = '\0';
		}
	}
}

void host_environment::add_files_from_directory_to_tpa_list(_In_z_ char* targetPath, _In_reads_(countExtensions) char** rgTPAExtensions, int countExtensions)
{
	*this->log << W("Adding assemblies from ") << targetPath << W(" to the TPA list") << logger::endl;
	char assemblyPath[MAX_LONGPATH];

	for (int iExtension = 0; iExtension < countExtensions; iExtension++)
	{
		strcpy(assemblyPath, targetPath);

		const size_t dirLength = strlen(targetPath);
		char* const fileNameBuffer = assemblyPath + dirLength;
		const size_t fileNameBufferSize = MAX_LONGPATH - dirLength;

		strcat(assemblyPath, rgTPAExtensions[iExtension]);
		WIN32_FIND_DATA data;
		HANDLE findHandle = FindFirstFile(assemblyPath, &data);

		if (findHandle != INVALID_HANDLE_VALUE) {
			do {
				if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					// It seems that CoreCLR doesn't always use the first instance of an assembly on the TPA list (ni's may be preferred
					// over il, even if they appear later). So, only include the first instance of a simple assembly name to allow
					// users the opportunity to override Framework assemblies by placing dlls in %CORE_LIBRARIES%

					// ToLower for case-insensitive comparisons
					char* fileNameChar = data.cFileName;
					while (*fileNameChar)
					{
						*fileNameChar = (char)tolower((int)*fileNameChar);
						fileNameChar++;
					}

					// Remove extension
					char fileNameWithoutExtension[MAX_LONGPATH];
					strcpy(fileNameWithoutExtension, data.cFileName);

					this->remove_extension_and_ni(fileNameWithoutExtension);

					// Add to the list if not already on it
					if (!this->tpa_list_contains_file(fileNameWithoutExtension, rgTPAExtensions, countExtensions))
					{
						const size_t fileLength = strlen(data.cFileName);
						const size_t assemblyPathLength = dirLength + fileLength;
						strcpy(fileNameBuffer, data.cFileName);
						this->tpa_list.append(assemblyPath, assemblyPathLength);
						this->tpa_list.append(";", 1);
					}
					else
					{
						*this->log << W("Not adding ") << targetPath << data.cFileName << W(" to the TPA list because another file with the same name is already present on the list") << logger::endl;
					}
				}
			} while (0 != FindNextFile(findHandle, &data));

			FindClose(findHandle);
		}
	}
}

// Returns the semicolon-separated list of paths to runtime dlls that are considered trusted.
// On first call, scans the coreclr directory for dlls and adds them all to the list.
const char * host_environment::get_tpa_list() {
	if (!this->tpa_list.c_str()) {
		char *rgTPAExtensions[] = {
			"*.ni.dll",		// Probe for .ni.dll first so that it's preferred if ni and il coexist in the same dir
			"*.dll",
			"*.ni.exe",
			"*.exe",
		};

		// Add files from %CORE_LIBRARIES% if specified
		char * coreLibraries;
		coreLibraries = getenv("CORE_LIBRARIES");

		if (coreLibraries)
		{
			strcat(coreLibraries, "\\");
			this->add_files_from_directory_to_tpa_list(coreLibraries, rgTPAExtensions, _countof(rgTPAExtensions));
		}
		else
		{
			*this->log << W("CORE_LIBRARIES not set; skipping") << logger::endl;
			*this->log << W("You can set the environment variable CORE_LIBRARIES to point to a") << logger::endl;
			*this->log << W("path containing additional platform assemblies,") << logger::endl;
		}

		this->add_files_from_directory_to_tpa_list(this->core_clr_directory_path, rgTPAExtensions, _countof(rgTPAExtensions));
	}

	return this->tpa_list.c_str();
}

// Returns the path to the host module
const char * host_environment::get_host_path() const {
	return this->host_path;
}

// Returns the path to the host module
const char * host_environment::get_host_exe_name() const {
	return this->host_exe_name;
}


// Returns the ICLRRuntimeHost2 instance, loading it from CoreCLR.dll if necessary, or nullptr on failure.
ICLRRuntimeHost2* host_environment::get_clr_runtime_host() {
	if (!this->clr_runtime_host) {

		if (!this->core_clr_module) {
			*this->log << W("Unable to load ") << core_clr_dll << logger::endl;
			return nullptr;
		}

		*this->log << W("Finding GetCLRRuntimeHost(...)") << logger::endl;

		FnGetCLRRuntimeHost pfnGetCLRRuntimeHost =
			(FnGetCLRRuntimeHost)::GetProcAddress(this->core_clr_module, "GetCLRRuntimeHost");

		if (!pfnGetCLRRuntimeHost) {
			*this->log << W("Failed to find function GetCLRRuntimeHost in ") << core_clr_dll << logger::endl;
			return nullptr;
		}

		*this->log << W("Calling GetCLRRuntimeHost(...)") << logger::endl;

		HRESULT hr = pfnGetCLRRuntimeHost(IID_ICLRRuntimeHost2, (IUnknown**)&this->clr_runtime_host);
		if (FAILED(hr)) {
			*this->log << W("Failed to get ICLRRuntimeHost2 interface. ERRORCODE: ") << hr << logger::endl;
			return nullptr;
		}
	}

	return this->clr_runtime_host;
}
