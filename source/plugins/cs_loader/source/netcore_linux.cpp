#include <cs_loader/netcore_linux.h>

netcore_linux::netcore_linux()
{
	this->runtimePath.append(getenv("CORE_ROOT"));
	getcwd(this->appPath, MAX_LONGPATH);
	this->domainId = 0;
}

netcore_linux::~netcore_linux()
{
	//this->stop();
}

bool netcore_linux::ConfigAssemblyName() {

	std::cout << "absoluteRuntime: " << this->runtimePath << std::endl;

	cout << "absoluteAppPath: " << this->appPath << endl;

	this->managedAssemblyFullName.append(this->appPath);
	this->managedAssemblyFullName.append("/");
	this->managedAssemblyFullName.append(this->loader_dll);

	cout << "absoluteLoaderDll: " << this->managedAssemblyFullName << endl;

	this->nativeDllSearchDirs.append(this->appPath);
	this->nativeDllSearchDirs.append(":");
	this->nativeDllSearchDirs.append(this->runtimePath);

	AddFilesFromDirectoryToTpaList(this->runtimePath, tpaList);

	return true;
}

bool netcore_linux::CreateHost() {

	dynlink handle = dynlink_load(this->runtimePath.c_str(), this->coreClrLibName.c_str(), DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

	if (handle == NULL) {
		return false;
	}

	dynlink_symbol_addr dynlink_coreclr_initialize;
	dynlink_symbol_addr dynlink_coreclr_shutdown;
	dynlink_symbol_addr dynlink_coreclr_create_delegate;

	dynlink_symbol(handle, "coreclr_initialize", &dynlink_coreclr_initialize);
	dynlink_symbol(handle, "coreclr_shutdown", &dynlink_coreclr_shutdown);
	dynlink_symbol(handle, "coreclr_create_delegate", &dynlink_coreclr_create_delegate);

	if (dynlink_coreclr_initialize == NULL) {
		std::cout << "coreclr_initialize fail " << std::endl;
		return false;
	}

	//this->dl = dynamicLinker::dynamicLinker::make_new(this->absoluteLibPath);
	//auto coreclr_initialize = dl->getFunction<coreclrInitializeFunction>("coreclr_initialize");
	//auto coreclr_shutdown = dl->getFunction<coreclrShutdownFunction>("coreclr_shutdown");
	//auto coreclr_create_delegate = dl->getFunction<coreclrCreateDelegateFunction>("coreclr_create_delegate");

	this->coreclr_initialize = (coreclrInitializeFunction*)dynlink_coreclr_initialize;
	this->coreclr_shutdown = (coreclrShutdownFunction*)dynlink_coreclr_shutdown;
	this->coreclr_create_delegate = (coreclrCreateDelegateFunction*)dynlink_coreclr_create_delegate;
	
	if (this->coreclr_initialize == NULL) {
		std::cout << "coreclr_initialize fail " << std::endl;
		return false;
	}

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
		std::cerr << "ERROR! coreclr_initialize status: 0x" << std::hex << status << std::endl;
		return false;
	}
	/*
	try {
		// create delegate to our entry point
		status = (*this->coreclr_create_delegate)(
			hostHandle,
			domainId,
			this->assemblyName,
			this->className,
			this->delegateLoadC,
			(void**)&this->coreLoadC);
	}
	catch (dynamicLinker::dynamicLinkerException e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	*/

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
	status = (*coreclr_create_delegate)(
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
		std::cerr << "ERROR! stop status: 0x" << std::hex << status << std::endl;
	}
}