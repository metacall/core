/*
 *	Loader Library by Parra Studios
 *	A plugin for loading crystal code at run-time into a process.
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

#include <cr_loader/cr_loader_impl.h>
#include <cr_loader/cr_loader_impl_main.h>

#include <loader/loader_impl.h>
#include <loader/loader_path.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <adt/adt_vector.h>

#include <log/log.h>

/* TODO: Implement Crystal loader wrapper for using cr_loader.cr functions and interfacing with the core */
