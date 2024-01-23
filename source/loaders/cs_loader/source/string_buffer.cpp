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

#include <cs_loader/string_buffer.h>
#include <wchar.h>

string_buffer::string_buffer() :
	capacity(0), buffer(nullptr), length(0)
{
}

string_buffer::~string_buffer()
{
	delete[] this->buffer;
}

const wchar_t *string_buffer::c_str() const
{
	return this->buffer;
}

void string_buffer::append(const wchar_t *str, size_t strLen)
{
	if (!this->buffer)
	{
		this->buffer = new wchar_t[this->default_size];
		this->capacity = this->default_size;
	}

	if (this->length + strLen + 1 > this->capacity)
	{
		size_t newCapacity = this->capacity * 2;
		wchar_t *newBuffer = new wchar_t[newCapacity];
		wcsncpy_s(newBuffer, newCapacity, this->buffer, this->length);
		delete[] this->buffer;
		this->buffer = newBuffer;
		this->capacity = newCapacity;
	}

	wcsncpy_s(this->buffer + this->length, this->capacity - this->length, str, strLen);
	this->length += strLen;
}
