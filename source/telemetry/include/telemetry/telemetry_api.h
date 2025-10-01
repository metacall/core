/*
 *	Telemetry Library by MetaCall Inc.
 *	A library for logging and telemetry collection in MetaCall runtime.
 *
 *	Copyright (C) 2025 MetaCall Inc., Dhiren Mhatre
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TELEMETRY_API_H
#define TELEMETRY_API_H 1

/* -- Headers -- */

#include <portability/portability.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#if defined(_WIN32) || defined(_WIN64)
	#ifdef TELEMETRY_EXPORTS
		#define TELEMETRY_API __declspec(dllexport)
	#else
		#define TELEMETRY_API __declspec(dllimport)
	#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
	#define TELEMETRY_API __attribute__((visibility("default")))
#else
	#define TELEMETRY_API
#endif

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_API_H */
