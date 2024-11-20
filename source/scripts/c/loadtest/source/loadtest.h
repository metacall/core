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

#include <cstdint>

typedef struct
{
	uint32_t i;
	double d;
} pair;

typedef struct
{
	uint32_t size;
	pair *pairs;
} pair_list;

EXPORT long call_cpp_func(void);

EXPORT int pair_list_init(pair_list **t);

EXPORT double pair_list_value(pair_list *t, uint32_t id);

EXPORT void pair_list_destroy(pair_list *t);

EXPORT void modify_int_ptr(long *i);

#ifdef __cplusplus
}
#endif

#endif /* LIB_LOAD_TEST_H */
