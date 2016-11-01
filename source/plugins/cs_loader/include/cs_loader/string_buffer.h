#pragma once
#ifndef _STRING_BUFFER_H_
#define _STRING_BUFFER_H_

#include <stdio.h>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>


class string_buffer
{
private:
	std::stringstream buffer;

	string_buffer(const string_buffer&);
	string_buffer& operator =(const string_buffer&);
public:
	string_buffer();

	~string_buffer();

	const char* c_str() const;

	void append(const char* str, size_t strLen);
};

#endif