#
#	Loader Library by Parra Studios
#	A plugin for loading crystal code at run-time into a process.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

require './main.cr'

fun cr_loader_impl_initialize(impl : Void*, config : Void*, host : Void*) : Void*
	# TODO: Review this, check NodeJS implementation for obtaining process name
	crystal_library_init(1, 'metacall')

	return nil
end

fun cr_loader_impl_execution_path(impl : Void*, path : LibC::Char*) : LibC::Int
	return 0
end

fun cr_loader_impl_load_from_file(impl : Void*, paths : LibC::Char**, size : LibC::SizeT) : Void*
	return nil
end

fun cr_loader_impl_load_from_memory(impl : Void*, name : LibC::Char*, buffer : LibC::Char*, size : LibC::SizeT) : Void*
	return nil
end

fun cr_loader_impl_load_from_package(impl : Void*, path : LibC::Char*) : Void*
	return nil
end

fun cr_loader_impl_clear(impl : Void*, handle : Void*) : LibC::Int
	return 0
end

fun cr_loader_impl_discover(impl : Void*, handle : Void*, ctx : Void*) : LibC::Int
	return 0
end

fun cr_loader_impl_destroy(impl : Void*) : LibC::Int
	return 0
end
