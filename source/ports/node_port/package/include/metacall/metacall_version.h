/*
 *	CMake Versioning Utility by Parra Studios
 *	A template for generating versioning utilities.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_VERSION_H
#define METACALL_VERSION_H 1

/* -- Headers -- */

#include <version/version_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Definitions -- */

#define METACALL_PROJECT_NAME            "MetaCall"
#define METACALL_PROJECT_DESCRIPTION     "A library for providing inter-language foreign function interface calls"

#define METACALL_AUTHOR_ORGANIZATION     "MetaCall Inc."
#define METACALL_AUTHOR_DOMAIN           "https://metacall.io/"
#define METACALL_AUTHOR_MAINTAINER       "vic798@gmail.com"

#define METACALL_VERSION_MAJOR           "0"
#define METACALL_VERSION_MAJOR_ID         0
#define METACALL_VERSION_MINOR           "1"
#define METACALL_VERSION_MINOR_ID         1
#define METACALL_VERSION_PATCH           "0"
#define METACALL_VERSION_PATCH_ID         0
#define METACALL_VERSION_REVISION        "000000000000"

#define METACALL_VERSION                 "0.1.0"
#define METACALL_NAME_VERSION            "MetaCall v0.1.0 (000000000000)"

/* -- Member Data -- */

struct MetaCall_version_type
{
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
	const char * revision;
	const char * str;
	const char * name;
};

/* -- Methods -- */

/**
*  @brief
*    Provide the module version struct
*
*  @return
*    Static struct containing unpacked version
*/
VERSION_API const void * MetaCall_version(void);

/**
*  @brief
*    Provide the module version hexadecimal value
*    with format 0xMMIIPPPP where M is @major,
*    I is @minor and P is @patch
*
*  @param[in] major
*    Unsigned integer representing major version
*
*  @param[in] minor
*    Unsigned integer representing minor version
*
*  @param[in] patch
*    Unsigned integer representing patch version
*
*  @return
*    Hexadecimal integer containing packed version
*/
VERSION_API uint32_t MetaCall_version_hex_make(unsigned int major, unsigned int minor, unsigned int patch);

/**
*  @brief
*    Provide the module version hexadecimal value
*    with format 0xMMIIPPPP where M is major,
*    I is minor and P is patch
*
*  @return
*    Hexadecimal integer containing packed version
*/
VERSION_API uint32_t MetaCall_version_hex(void);

/**
*  @brief
*    Provide the module version string
*
*  @return
*    Static string containing module version
*/
VERSION_API const char * MetaCall_version_str(void);

/**
*  @brief
*    Provide the module version revision string
*
*  @return
*    Static string containing module version revision
*/
VERSION_API const char * MetaCall_version_revision(void);

/**
*  @brief
*    Provide the module version name
*
*  @return
*    Static string containing module version name
*/
VERSION_API const char * MetaCall_version_name(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_VERSION_H */
