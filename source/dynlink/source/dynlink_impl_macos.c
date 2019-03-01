
/* -- Headers -- */

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <dlfcn.h>

/* -- Methods -- */

const char * dynlink_impl_interface_extension_macos(void)
{
	static const char extension_macos[0x03] = "dylib";

	return extension_macos;
}

void dynlink_impl_interface_get_name_macos(dynlink handle, dynlink_name_impl name_impl, size_t length)
{
	strncpy(name_impl, "lib", length);

	strncat(name_impl, dynlink_get_name(handle), length);

	strncat(name_impl, ".", length);

	strncat(name_impl, dynlink_impl_extension(), length);
}

dynlink_impl dynlink_impl_interface_load_macos(dynlink handle)
{
	CFStringRef LibraryNameString = CFStringCreateWithCString(kCFAllocatorDefault, Name, kCFStringEncodingASCII);

	impl->BundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, LibraryNameString, kCFURLPOSIXPathStyle, true);

	impl->Bundle = CFBundleCreate(kCFAllocatorDefault, impl->BundleURL);

	CFRelease(LibraryNameString);

	if (impl != NULL)
	{
		return (dynlink_impl)impl;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "DynLink error: %s", dlerror());

	return NULL;
}

int dynlink_impl_interface_symbol_macos(dynlink handle, dynlink_impl impl, dynlink_symbol_name name, dynlink_symbol_addr * addr)
{
	CFStringRef SymbolString = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);

	*addr = CFBundleGetFunctionPointerForName(impl->Bundle, SymbolString);
	
	CFRelease(SymbolString);

	return (*addr == NULL);
}

int dynlink_impl_interface_unload_macos(dynlink handle, dynlink_impl impl)
{
	CFRelease(impl->Bundle);
	CFRelease(impl->BundleURL);

	return true;
}

dynlink_impl_interface dynlink_impl_interface_singleton_macos(void)
{
	static struct dynlink_impl_interface_type impl_interface_macos =
	{
		&dynlink_impl_interface_extension_macos,
		&dynlink_impl_interface_get_name_macos,
		&dynlink_impl_interface_load_macos,
		&dynlink_impl_interface_symbol_macos,
		&dynlink_impl_interface_unload_macos
	};

	return &impl_interface_macos;
}
