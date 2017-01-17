#pragma once
#ifndef _NETCORE_WIN_H_
#define _NETCORE_WIN_H_
#include "defs.h"
#include "netcore.h"

typedef class host_environment;
typedef class ICLRRuntimeHost2;
typedef class logger;

class netcore_win : public netcore
{
private:
	logger * log;
	host_environment * core_environment;
	ICLRRuntimeHost2 * host;
	wchar_t managedAssemblyFullName[MAX_LONGPATH] = W("");
	wchar_t appPath[MAX_LONGPATH] = W("");
	wchar_t appNiPath[MAX_LONGPATH * 2] = W("");
	wchar_t nativeDllSearchDirs[MAX_LONGPATH * 3];
	unsigned long domain_id;

	bool config_assembly_name();

	bool create_host();

	bool create_delegate(const wchar_t * delegate_name, void** func);

	bool load_main();

public:
	netcore_win();
	~netcore_win();

	bool start();

	void stop();
};


#endif