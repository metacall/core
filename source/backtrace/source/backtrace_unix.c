/*
 *	Backtrace Library by Parra Studios
 *	A cross-platform library for supporting SEGV catching and backtracing.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <backtrace/backtrace.h>

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
	#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	#include <sys/ucontext.h>
#else
	#include <ucontext.h>
#endif
#include <unistd.h>

/* -- Definitions -- */

#define BACKTRACE_SIZE 100

/* -- Methods -- */

static void backtrace_handler(int sig_num, siginfo_t *info, void *ucontext)
{
	void *array[BACKTRACE_SIZE];
	int size = backtrace(array, 10);
	char **strings = backtrace_symbols(array, size);

	(void)ucontext;

	if (strings != NULL)
	{
		int iterator;

		printf("%s catched by backtrace library (at %p):\n", strsignal(sig_num), info->si_addr);

		for (iterator = 0; iterator < size; ++iterator)
		{
			printf("%s\n", strings[iterator]);
		}

		fflush(stdout);
		free(strings);
	}

	exit(EXIT_FAILURE);
}

int backtrace_initialize(void)
{
	struct sigaction sigact;

	sigact.sa_sigaction = backtrace_handler;
	sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	return sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL);
}

int backtrace_destroy(void)
{
	return sigaction(SIGSEGV, (const struct sigaction *)SIG_DFL, (struct sigaction *)NULL);
}
