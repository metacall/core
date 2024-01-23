#
#	Loader Library by Parra Studios
#	A plugin for loading crystal code at run-time into a process.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

lib LibMetaCall
	# Loader
	fun loader_impl_get(impl : Void*) : Void*
	fun loader_initialization_register(impl : Void*) : Void
	fun loader_unload_children(impl : Void*, destroy_children : int) : Void

	# Log
	# TODO: log_write is a macro that must be reimplemented properly (log_write_impl, log_write_impl_va)
	# fun log_write()

end
