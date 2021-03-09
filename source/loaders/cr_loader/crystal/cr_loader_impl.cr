#
#	Loader Library by Parra Studios
#	A plugin for loading crystal code at run-time into a process.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

require './metacall.cr'
require './main.cr'

struct CrystalLoaderImpl
	# TODO
end

fun cr_loader_impl_initialize(impl : Void*, config : Void*) : Void*
	# TODO: Review this, check NodeJS implementation for obtaining process name
	crystal_library_init(1, 'metacall')

	cr_impl = CrystalLoaderImpl.new

	# TODO: Initialize cr_impl properly

	# Register current loader initialization order
	LibMetaCall.loader_initialization_register(impl);

	return Box.box(cr_impl)
end

fun cr_loader_impl_execution_path(impl : Void*, path : LibC::Char*) : LibC::Int
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return 0
end

fun cr_loader_impl_load_from_file(impl : Void*, paths : LibC::Char**, size : LibC::SizeT) : Void*
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return nil
end

fun cr_loader_impl_load_from_memory(impl : Void*, name : LibC::Char*, buffer : LibC::Char*, size : LibC::SizeT) : Void*
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return nil
end

fun cr_loader_impl_load_from_package(impl : Void*, path : LibC::Char*) : Void*
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return nil
end

fun cr_loader_impl_clear(impl : Void*, handle : Void*) : LibC::Int
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return 0
end

fun cr_loader_impl_discover(impl : Void*, handle : Void*, ctx : Void*) : LibC::Int
	cr_impl = Box(CrystalLoaderImpl).unbox(LibMetaCall.loader_impl_get(impl))

	return 0
end

fun cr_loader_impl_destroy(impl : Void*) : LibC::Int
	ptr = LibMetaCall.loader_impl_get(impl)

	# Destroy children loaded by this loader
	LibMetaCall.loader_unload_children()

	cr_impl = Box(CrystalLoaderImpl).unbox(ptr)

	# TODO: Do destruction of cr_impl

	GC.free(ptr)

	# TODO: crystal_library_destroy

	return 0
end
