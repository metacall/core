/*
 *	Threading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <threading/threading_mutex.h>

#include <semLib.h>

int threading_mutex_initialize(threading_mutex m)
{
	*m = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
	return (*m == NULL) ? 1 : 0;
}

int threading_mutex_lock(threading_mutex m)
{
	return (semTake(*m, WAIT_FOREVER) == OK) ? 0 : 1;
}

int threading_mutex_try_lock(threading_mutex m)
{
	return (semTake(*m, NO_WAIT) == OK) ? 0 : 1;
}

int threading_mutex_unlock(threading_mutex m)
{
	return (semGive(*m) == OK) ? 0 : 1;
}

int threading_mutex_destroy(threading_mutex m)
{
	return (semDelete(*m) == OK) ? 0 : 1;
}
