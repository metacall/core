/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
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

/* -- Headers -- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DELAYIMP_INSECURE_WRITABLE_HOOKS 1
#include <delayimp.h>

static FARPROC WINAPI node_loader_win32_delay_load(unsigned dliNotify, PDelayLoadInfo pdli);

#if (defined(DELAYLOAD_VERSION) && DELAYLOAD_VERSION >= 0x0200) || (defined(_DELAY_IMP_VER) && (_DELAY_IMP_VER >= 2))
	extern PfnDliHook __pfnDliFailureHook2 = node_loader_win32_delay_load;
#else
	extern PfnDliHook __pfnDliFailureHook = node_loader_win32_delay_load;
#endif

FARPROC WINAPI node_loader_win32_delay_load(unsigned dliNotify, PDelayLoadInfo pdli)
{
	FARPROC fp_module_register = NULL;

	if (dliNotify == dliFailGetProc)
	{
		LPCTSTR module_handle_lpctstr = "node.dll";

		HMODULE module_handle = GetModuleHandle(module_handle_lpctstr);

		fp_module_register = ::GetProcAddress(module_handle, pdli->dlp.szProcName);
	}

	return fp_module_register;
}
