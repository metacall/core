#include <cs_loader/netcore_linux.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <dynlink/dynlink.h>
#include <stdlib.h>
#include <unistd.h>
#include <log/log.h>
netcore_linux::netcore_linux()
{
	char * env;

	env = getenv("CORE_ROOT");
	if (env == nullptr) {
		log_write("metacall", LOG_LEVEL_DEBUG, "CORE_ROOT not defined");
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "CORE_ROOT is %s", env);
	}

	this->runtimePath.append(env);
	getcwd(this->appPath, MAX_LONGPATH);
	this->domainId = 0;
}

netcore_linux::~netcore_linux()
{
	//this->stop();
}

bool netcore_linux::ConfigAssemblyName() {

	this->absoluteLibPath += this->runtimePath + this->coreClrDll;

	//std::cout << "absoluteRuntime: " << this->runtimePath << std::endl;

//	std::cout << "absoluteLibPath: " << this->absoluteLibPath << std::endl;

//	cout << "absoluteAppPath: " << this->appPath << endl;

	this->managedAssemblyFullName.append(this->appPath);
	this->managedAssemblyFullName.append("/");
	this->managedAssemblyFullName.append(this->loader_dll);

	//	cout << "absoluteLoaderDll: " << this->managedAssemblyFullName << endl;

	this->nativeDllSearchDirs.append(this->appPath);
	this->nativeDllSearchDirs.append(":");
	this->nativeDllSearchDirs.append(this->runtimePath);

	this->add_files_from_directory_to_tpa_list(this->runtimePath, tpaList);

	return true;
}

bool netcore_linux::CreateHost() {
	this->dl_handle = dynlink_load(this->runtimePath.c_str(), this->coreClrDll.c_str(), DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_LOCAL);

	if (this->dl_handle == nullptr) {
		log_write("metacall", LOG_LEVEL_DEBUG, "can't load %slib%s.%s", this->runtimePath.c_str(), this->coreClrDll.c_str(), dynlink_extension());
		return false;
	}

	dynlink_symbol_addr addr;

	dynlink_symbol(this->dl_handle, "coreclr_initialize", &addr);

	this->coreclr_initialize = (coreclrInitializeFunction)addr;

	dynlink_symbol(this->dl_handle, "coreclr_shutdown", &addr);

	this->coreclr_shutdown = (coreclrShutdownFunction)addr;

	dynlink_symbol(this->dl_handle, "coreclr_create_delegate", &addr);

	this->coreclr_create_delegate = (coreclrCreateDelegateFunction)addr;

	const char *propertyKeys[] = {
		"TRUSTED_PLATFORM_ASSEMBLIES",
		"APP_PATHS",
		"APP_NI_PATHS",
		"NATIVE_DLL_SEARCH_DIRECTORIES",
		"AppDomainCompatSwitch"
	};

	const char *propertyValues[] = {
		tpaList.c_str(),
		appPath,
		appPath,
		nativeDllSearchDirs.c_str(),
		"UseLatestBehaviorWhenTFMNotSpecified"
	};

	int status = -1;

	// initialize coreclr
	string m;
	string mm;
	mm.append(getenv("_"));
	m.append(this->appPath);
	m.append(mm.substr(1, mm.length() - 1));

	status = (*this->coreclr_initialize)(
		m.c_str(),
		"metacall_cs_loader_container",
		sizeof(propertyKeys) / sizeof(propertyKeys[0]),
		propertyKeys,
		propertyValues,
		&hostHandle,
		&domainId
		);

	if (status < 0) {
		log_write("metacall", LOG_LEVEL_ERROR, "coreclr_initialize fail!");
		//std::cout << "ERROR! coreclr_initialize status: 0x" << std::hex << status << std::endl;
		return false;
	}

	if (!this->create_delegates()) {
		return false;
	}

	return true;
}
bool netcore_linux::LoadMain() {

	return true;
}

bool netcore_linux::create_delegate(const CHARSTRING * delegateName, void** funcs) {
	int status = -1;

	// create delegate to our entry point
	status = (*this->coreclr_create_delegate)(
		hostHandle,
		domainId,
		this->assembly_name,
		this->class_name,
		delegateName,
		funcs);

	if (status < 0) {
		std::cerr << "ERROR! CreateDelegate status: 0x" << std::hex << status << std::endl;
		return false;
	}

	return true;
}


void netcore_linux::stop() {
	int status = -1;

	status = (*this->coreclr_shutdown)(
		this->hostHandle,
		this->domainId);

	if (status < 0) {
		std::cout << "ERROR! stop status: 0x" << std::hex << status << std::endl;
	}

	dynlink_unload(this->dl_handle);
}

void netcore_linux::add_files_from_directory_to_tpa_list(std::string directory, std::string& tpaList) {

	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(directory.c_str())) == NULL) {
		std::cout << "Error(" << errno << ") opening " << directory << std::endl;
		return;
	}

	while ((dirp = readdir(dp)) != NULL) {

		if (dirp->d_type == DT_REG) {
			std::string path;
			path.append(directory);
			path.append(dirp->d_name);

			if (!path.compare(path.length() - 4, 4, ".dll")) {
				tpaList.append(path + ":");
			}
		}
	}

	closedir(dp);
}