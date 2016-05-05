/////////////////////////////////////////////////////////////////////////////
//  Argentum Online C by Parra Studios
//
//  A cross-platform mmorpg which keeps the original essence of
//  Argentum Online created by Pablo Ignacio Marquez (Gulfas Morgolock),
//  as combats, magics, guilds, although it has new implementations such as
//  3D graphics engine, new gameplay, and a better performance, among others.
//
//  Copyright (C) 2009-2015 Vicente Ferrer Garcia (Parra) - vic798@gmail.com
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/> or
//  <http://www.affero.org/oagpl.html>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <System/Unix/LibraryImpl.h>
#include <Memory/General.h>
#include <dlfcn.h>

////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Member data
////////////////////////////////////////////////////////////
struct LibraryImplHandleType
{
	void * Handle;
};

////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Get the library extension for Unix
////////////////////////////////////////////////////////////
const char * LibraryExtensionImpl()
{
    static const char LibraryExtensionImplUnix[0x04] = ".so";

	return LibraryExtensionImplUnix;
}
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Loads a library returning the handle to it in Unix platform
////////////////////////////////////////////////////////////
LibraryImplType LibraryLoadImpl(LibraryNameType Name, LibraryFlagsType Flags)
{
    struct LibraryImplHandleType * Impl = (struct LibraryImplHandleType *)MemoryAllocate(sizeof(struct LibraryImplHandleType));

    if (Impl)
    {
        integer Mode = 0;

        // Bind flags
        if (Flags & LIBRARY_FLAGS_BIND_LAZY)
        {
            Mode |= RTLD_LAZY;
        }

        if (Flags & LIBRARY_FLAGS_BIND_LOCAL)
        {
            Mode |= RTLD_LOCAL;
        }

        if (Flags & LIBRARY_FLAGS_BIND_GLOBAL)
        {
            Mode |= RTLD_GLOBAL;
        }

        // Create and open handle
        Impl->Handle = dlopen(Name, Mode);

        // Return implementation on success
        if (Impl->Handle)
        {
            return (LibraryImplType)Impl;
        }

        // Destroy implementation on error
        MemoryDeallocate(Impl);
    }

	return NULL;
}

////////////////////////////////////////////////////////////
/// Unloads a library by its handle in Unix platform
////////////////////////////////////////////////////////////
bool LibraryUnloadImpl(LibraryImplType Impl)
{
    bool Result = false;

    if (Impl)
    {
        struct LibraryImplHandleType * ImplHandle = Impl;

        // Close handle
        if (dlclose(ImplHandle->Handle))
        {
            Result = true;
        }

        // Destroy implementation
        MemoryDeallocate(ImplHandle);
    }

	return Result;
}

////////////////////////////////////////////////////////////
/// Get a symbol from library in Unix platform
////////////////////////////////////////////////////////////
bool LibrarySymbolImpl(LibraryImplType Impl, LibrarySymbolNameType Name, LibrarySymbolType * Address)
{
    if (Impl)
    {
        struct LibraryImplHandleType * ImplHandle = Impl;

        // Retreive address from symbol and handle
        *Address = dlsym(ImplHandle->Handle, Name);

        return (*Address != NULL);
    }

    return false;
}
