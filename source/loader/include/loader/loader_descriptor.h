/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_DESCRIPTOR_H
#define LOADER_DESCRIPTOR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct loader_descriptor_version_type
{
	const uint32_t number; /**< Version number on unsigned integer (hexadecimal) */
	const char *str;	   /**< Version string (human readable) */

} * loader_descriptor_version;

typedef struct loader_descriptor_date_type
{
	const uint8_t day;
	const char *month;
	const uint32_t year;
} * loader_descriptor_date;

typedef struct loader_descriptor_copyright_type
{
	const char *authors;						   /**< Authors of the loader */
	const char *owners;							   /**< Owners of the loader */
	const char *webpage;						   /**< Web of the loader developers */
	struct loader_descriptor_date_type date;	   /**< Date in which the loader was created */
	const char *organization;					   /**< Organization name under loader is developed */
	const char *maintainers;					   /**< Current maintainers of the loader */
	struct loader_descriptor_version_type version; /**< Current version of the loader */
	const char *license;						   /**< License of the loader */

} * loader_descriptor_copyright;

typedef struct loader_descriptor_language_type
{
	const char *name;							   /**< Name of the language of the loader */
	struct loader_descriptor_version_type version; /**< Version of the current standard of the lenguage */

} * loader_descriptor_language;

typedef struct loader_descriptor_extension_type
{
	const char **list;	 /**< Default extensions supported by the technology */
	const uint32_t size; /**< Number of available default extensions */

} * loader_descriptor_extension;

typedef struct loader_descriptor_technology_type
{
	const char *name;							   /**< Name of technology used in the loader */
	const char *vendor;							   /**< Vendor which develops the technology used in the loader */
	const char *webpage;						   /**< Web of the technology */
	struct loader_descriptor_version_type version; /**< Version of the technology */
	struct loader_descriptor_extension_type ext;   /**< Describes default extensions supported by the loader */

} * loader_descriptor_technology;

typedef struct loader_descriptor_platform_type
{
	const char *architecture;	  /**< Architecture where loader was compiled */
	const char *compiler;		  /**< Compiler used to build the loader */
	const char *operative_system; /**< Operative system under loader was compiled */
	const char *family;			  /**< Platform family which operative system belongs to */

} * loader_descriptor_platform;

typedef struct loader_descriptor_type
{
	const char *name;						 /**< Loader name */
	const char *keyword;					 /**< Loader identifier */
	const char *description;				 /**< Loader description */
	loader_descriptor_copyright copyright;	 /**< Copyright information */
	loader_descriptor_language language;	 /**< Loader language information */
	loader_descriptor_technology technology; /**< Loader technology information */
	loader_descriptor_platform platform;	 /**< Platform information */

} * loader_descriptor;

#ifdef __cplusplus
}
#endif

#endif /* LOADER_DESCRIPTOR_H */
