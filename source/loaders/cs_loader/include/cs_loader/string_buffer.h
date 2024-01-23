/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef _STRING_BUFFER_H_
#define _STRING_BUFFER_H_

class string_buffer
{
private:
	static const int default_size = 4096;
	wchar_t *buffer;
	size_t capacity;
	size_t length;

	string_buffer(const string_buffer &);
	string_buffer &operator=(const string_buffer &);

public:
	string_buffer();

	~string_buffer();

	const wchar_t *c_str() const;

	void append(const wchar_t *str, size_t strLen);
};

#endif
