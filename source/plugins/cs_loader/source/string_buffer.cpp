#include <cs_loader/string_buffer.h>
#include <string.h>


string_buffer::string_buffer() : capacity(0), buffer(nullptr), length(0) {
}

string_buffer::~string_buffer()
{
}

const char* string_buffer::c_str() const
{
	if (this->buffer == nullptr) {
		return nullptr;
	}

	*(this->buffer + this->length) = '\0';
	return this->buffer;
}

void string_buffer::append(const char* str, size_t strLen)
{
	if (!this->buffer) {
		this->buffer = new char[this->default_size];
		this->capacity = this->default_size;
	}
	if (this->length + strLen + 1 > this->capacity) {
		size_t newCapacity = this->capacity * 2;
		char* newBuffer = new char[newCapacity];
		//strncpy_s(newBuffer, newCapacity, this->buffer, this->length);
		strncpy(newBuffer, this->buffer, this->length);
		delete[] this->buffer;
		this->buffer = newBuffer;
		this->capacity = newCapacity;
	}
	//strncpy_s(this->buffer + this->length, this->capacity - this->length, str, strLen);
	strncpy(this->buffer + this->length, str, strLen);
	this->length += strLen;
}