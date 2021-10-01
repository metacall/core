/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_ACCESSOR_H
#define REFLECT_ACCESSOR_H 1

#include <reflect/reflect_attribute_decl.h>
#include <reflect/reflect_constructor_decl.h>
#include <reflect/reflect_method_decl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* Depending on the class or object accessor type,
* if it is static, there will be an error when accessing
* an attribute that was not registered, otherwise
* it will allow setting or getting the attribute dynamically
*/
enum accessor_type_id
{
	ACCESSOR_TYPE_STATIC = 0,
	ACCESSOR_TYPE_DYNAMIC = 1
};

struct accessor_type
{
	enum accessor_type_id id;

	/*
	* In addition to attribute, there is constructor and method
	* also included for future uses
	*/
	union
	{
		const char *key;
		constructor ctor;
		attribute attr;
		method m;
	} data;
};

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_ACCESSOR_H */
