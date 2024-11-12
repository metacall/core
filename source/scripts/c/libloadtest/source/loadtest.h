#ifndef LIB_LOAD_TEST_H
#define LIB_LOAD_TEST_H 1

#if defined(WIN32) || defined(_WIN32)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT long call_cpp_func(void);

#ifdef __cplusplus
}
#endif

#endif /* LIB_LOAD_TEST_H */
