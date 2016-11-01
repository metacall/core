#pragma once
#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <stdio.h>
#include <ostream>
#include <string>
#include <sstream>
#include <cs_loader/defs.h>

class logger {
	bool is_enabled;
	std::wstringstream buffer;
public:
	logger() :
		is_enabled(true)
	{
	}

	~logger() { }

	// Enables output from the logger
	void enable();

	// Disables output from the logger
	void disable();

	logger& operator<< (bool val);
	logger& operator<< (short val);
	logger& operator<< (unsigned short val);
	logger& operator<< (int val);
	logger& operator<< (unsigned int val);

	logger& operator<< (long val);
	logger& operator<< (unsigned long val);

	logger& operator<< (float val);
	logger& operator<< (double val);
	logger& operator<< (long double val);
	logger& operator<< (const char* val);
	logger& operator<< (const wchar_t* val);
	logger& operator<< (logger& (*pf)(logger&));
	static logger& endl(logger& log);
	void flush();
private:
};

#endif