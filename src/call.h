#ifndef CALL_H
#define CALL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

void * metacall(char * func, ...);

#ifdef __cplusplus
}
#endif

#endif // CALL_H
