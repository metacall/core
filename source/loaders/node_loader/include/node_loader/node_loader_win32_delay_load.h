/*
 *	Loader Library by Parra Studios
 *	A plugin for loading nodejs code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <stdlib.h>
#include <string.h>

inline void *node_loader_hook_import_address_table(const char *module_name, const char *function_name, void *hook)
{
	LPVOID image_base = GetModuleHandleA(module_name);
	PIMAGE_DOS_HEADER dos_headers = (PIMAGE_DOS_HEADER)image_base;

	if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}

	PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD_PTR)image_base + dos_headers->e_lfanew);

	if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		return NULL;
	}

	IMAGE_DATA_DIRECTORY *imports_directory = &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (imports_directory->Size == 0 || imports_directory->VirtualAddress == 0)
	{
		return NULL;
	}

	PIMAGE_IMPORT_DESCRIPTOR import_descriptor = (PIMAGE_IMPORT_DESCRIPTOR)(imports_directory->VirtualAddress + (DWORD_PTR)image_base);

	for (; import_descriptor->FirstThunk != NULL; ++import_descriptor)
	{
		PIMAGE_THUNK_DATA original_first_thunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)image_base + import_descriptor->OriginalFirstThunk); // Image thunk data names
		PIMAGE_THUNK_DATA first_thunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)image_base + import_descriptor->FirstThunk);					 // Image thunk data address

		for (; original_first_thunk->u1.AddressOfData != NULL; ++original_first_thunk, ++first_thunk)
		{
			if ((original_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
			{
				PIMAGE_IMPORT_BY_NAME func = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)image_base + original_first_thunk->u1.AddressOfData);

				if (strcmp(func->Name, function_name) == 0)
				{
					LPVOID import_func_load_address = (LPVOID)(&first_thunk->u1.Function);
					DWORD old_page_protect, dummy_old_page_protect;
					VirtualProtect(import_func_load_address, sizeof(void *), PAGE_EXECUTE_READWRITE, &old_page_protect);

					memcpy(import_func_load_address, &hook, sizeof(hook));

					VirtualProtect(import_func_load_address, sizeof(void *), old_page_protect, &dummy_old_page_protect);

					return (void *)import_func_load_address;
				}
			}
		}
	}
}
