#include <cs_loader/logger.h>

#include <ostream>
#include <iostream>

void logger::enable() {
	this->is_enabled = true;
}

void logger::disable() {
	this->is_enabled = false;
}

logger& logger::operator<< (bool val) {
	if (this->is_enabled) {
		if (val) {
			this->buffer << val;
		}
		else {
			this->buffer << val;
		}
	}
	return *this;
}

logger& logger::operator<< (int val) {

	if (this->is_enabled) {
		this->buffer << val;
	}

	return *this;
}

logger& logger::operator<< (long val) {
	if (this->is_enabled) {
		this->buffer << val;
	}
	return *this;
}

logger& logger::operator<< (unsigned long val) {
	if (this->is_enabled) {
		this->buffer << val;
	}
	return *this;
}
logger& logger::operator<< (const char *val) {
	if (this->is_enabled) {
		this->buffer << val;
	}
	return *this;
}

logger& logger::operator<< (const wchar_t *val) {
	if (this->is_enabled) {
		this->buffer << val;
	}
	return *this;
}

logger& logger::operator<< (logger& (*pf)(logger&)) {
	if (this->is_enabled) {
		return pf(*this);
	}
	else {
		return *this;
	}
}

// Manipulators

void logger::flush() {
	std::wcout << this->buffer.str().c_str() << std::endl;
	this->buffer.clear();
}

// Newline
logger& logger::endl(logger& log) {
	if (log.is_enabled) {
		log.flush();
	}

	return log;
}

