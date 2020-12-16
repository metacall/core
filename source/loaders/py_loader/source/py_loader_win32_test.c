/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

 // To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
 // and compile with -DPSAPI_VERSION=1

int PrintModules()
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;
	DWORD processID = GetCurrentProcessId();

	// Print the process identifier.

	printf("\nProcess ID: %u\n", processID);

	// Get a handle to the process.

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	if (NULL == hProcess)
		return 1;

	// Get a list of all the modules in this process.

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.

			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				// Print the module name and handle value.
				if (strcmp(szModName, "E:\\buildw\\Debug\\py_loaderd.dll") == 0)
				{
					_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);

					void *(*ptr)(void) = GetProcAddress(hMods[i], "PyInit_py_loaderd");

					// Removed by now
					// printf("Addr: %p\n", ptr());
				}
			}
		}
	}

	// Release the handle to the process.

	CloseHandle(hProcess);

	return 0;
}
