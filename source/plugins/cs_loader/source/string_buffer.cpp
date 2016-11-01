#include <cs_loader/string_buffer.h>



string_buffer::string_buffer() {
}

string_buffer::~string_buffer()
{
}

const char* string_buffer::c_str() const
{
	return this->buffer.str().c_str();
}

void string_buffer::append(const char* str, size_t strLen)
{
	(void)strLen;
	this->buffer << str;
}