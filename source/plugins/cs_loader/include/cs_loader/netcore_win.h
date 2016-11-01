#pragma once
#ifndef _NETCORE_WIN_H_
#define _NETCORE_WIN_H_
#include <cs_loader/defs.h>
#include <cs_loader/logger.h>
#include <cs_loader/host_environment.h>
#include <cs_loader/netcore.h>
#include <pal/prebuilt/inc/mscoree.h>

//typedef class host_environment;
//typedef class ICLRRuntimeHost2;
//typedef class logger;

class netcore_win : public netcore
{
private:
	logger * log;
	host_environment * core_environment;
	ICLRRuntimeHost2 * host;
	char managedAssemblyFullName[MAX_LONGPATH] = "";
	char appPath[MAX_LONGPATH] = "";
	char appNiPath[MAX_LONGPATH * 2] = "";
	char nativeDllSearchDirs[MAX_LONGPATH * 3];
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