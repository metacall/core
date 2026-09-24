/*
 *	Serial Library by Parra Studios
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall_version.h>

#include <grpc_serial/grpc_serial.h>
#include <grpc_serial/grpc_serial_impl.h>

/* -- Methods -- */

serial_interface grpc_serial_impl_interface_singleton(void)
{
	static struct serial_interface_type interface_instance_grpc = {
		&grpc_serial_impl_extension,
		&grpc_serial_impl_initialize,
		&grpc_serial_impl_serialize,
		&grpc_serial_impl_deserialize,
		&grpc_serial_impl_destroy
	};

	return &interface_instance_grpc;
}

const char *grpc_serial_print_info(void)
{
	static const char grpc_serial_info[] =
		"gRPC Serial Plugin " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"
#ifdef GRPC_SERIAL_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif
		"\n";

	return grpc_serial_info;
}
