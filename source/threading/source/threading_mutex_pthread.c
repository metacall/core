/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <threading/threading_mutex.h>

int threading_mutex_initialize(threading_mutex m)
{
	return pthread_mutex_init(m, NULL);
}

int threading_mutex_lock(threading_mutex m)
{
	return pthread_mutex_lock(m);
}

int threading_mutex_try_lock(threading_mutex m)
{
	return pthread_mutex_trylock(m);
}

int threading_mutex_unlock(threading_mutex m)
{
	return pthread_mutex_unlock(m);
}

int threading_mutex_destroy(threading_mutex m)
{
	return pthread_mutex_destroy(m);
}
