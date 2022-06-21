#ifndef EXTENSIONSUM_H
#define EXTENSIONSUM_H 1

#ifdef _WIN32
	#define EXTENSIONSUM_API __declspec(dllexport)
#else
	#define EXTENSIONSUM_API __attribute__((visibility("default")))
#endif

EXTENSIONSUM_API void *extensionsum(void);

#endif /* EXTENSIONSUM_H */
