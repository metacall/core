#ifndef EXTENSIONSUM_H
#define EXTENSIONSUM_H 1

#if defined(WIN32) || defined(_WIN32)
	#define EXTENSIONSUM_API __declspec(dllexport)
#else
	#define EXTENSIONSUM_API __attribute__((visibility("default")))
#endif

EXTENSIONSUM_API void extensionsum(void *loader, void *context);

#endif /* EXTENSIONSUM_H */
