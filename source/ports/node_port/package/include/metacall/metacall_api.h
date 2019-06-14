
#ifndef METACALL_API_H
#define METACALL_API_H

#ifdef METACALL_STATIC_DEFINE
#  define METACALL_API
#  define METACALL_NO_EXPORT
#else
#  ifndef METACALL_API
#    ifdef metacall_EXPORTS
        /* We are building this library */
#      define METACALL_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define METACALL_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef METACALL_NO_EXPORT
#    define METACALL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef METACALL_DEPRECATED
#  define METACALL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef METACALL_DEPRECATED_EXPORT
#  define METACALL_DEPRECATED_EXPORT METACALL_API METACALL_DEPRECATED
#endif

#ifndef METACALL_DEPRECATED_NO_EXPORT
#  define METACALL_DEPRECATED_NO_EXPORT METACALL_NO_EXPORT METACALL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef METACALL_NO_DEPRECATED
#    define METACALL_NO_DEPRECATED
#  endif
#endif

#endif
