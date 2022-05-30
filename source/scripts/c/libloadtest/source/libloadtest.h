#ifndef LIB_LOAD_TEST_H
#define LIB_LOAD_TEST_H 1

#ifdef _WIN32
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
#endif

EXPORT long call_cpp_func(void);

#endif LIB_LOAD_TEST_H
