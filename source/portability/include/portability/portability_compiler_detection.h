/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PORTABILITY_COMPILER_DETECTION_H
#define PORTABILITY_COMPILER_DETECTION_H 1

/* TODO: This needs to be implemented properly, including another file for architecture and operative system detection */

#define PORTABILITY_COMPILER_IS_Comeau		0
#define PORTABILITY_COMPILER_IS_Intel		0
#define PORTABILITY_COMPILER_IS_PathScale	0
#define PORTABILITY_COMPILER_IS_Embarcadero 0
#define PORTABILITY_COMPILER_IS_Borland		0
#define PORTABILITY_COMPILER_IS_Watcom		0
#define PORTABILITY_COMPILER_IS_OpenWatcom	0
#define PORTABILITY_COMPILER_IS_SunPro		0
#define PORTABILITY_COMPILER_IS_HP			0
#define PORTABILITY_COMPILER_IS_Compaq		0
#define PORTABILITY_COMPILER_IS_zOS			0
#define PORTABILITY_COMPILER_IS_XLClang		0
#define PORTABILITY_COMPILER_IS_XL			0
#define PORTABILITY_COMPILER_IS_VisualAge	0
#define PORTABILITY_COMPILER_IS_PGI			0
#define PORTABILITY_COMPILER_IS_Cray		0
#define PORTABILITY_COMPILER_IS_TI			0
#define PORTABILITY_COMPILER_IS_Fujitsu		0
#define PORTABILITY_COMPILER_IS_GHS			0
#define PORTABILITY_COMPILER_IS_SCO			0
#define PORTABILITY_COMPILER_IS_ARMCC		0
#define PORTABILITY_COMPILER_IS_AppleClang	0
#define PORTABILITY_COMPILER_IS_ARMClang	0
#define PORTABILITY_COMPILER_IS_Clang		0
#define PORTABILITY_COMPILER_IS_GNU			0
#define PORTABILITY_COMPILER_IS_MSVC		0
#define PORTABILITY_COMPILER_IS_ADSP		0
#define PORTABILITY_COMPILER_IS_IAR			0
#define PORTABILITY_COMPILER_IS_MIPSpro		0

#if defined(__COMO__)
	#undef PORTABILITY_COMPILER_IS_Comeau
	#define PORTABILITY_COMPILER_IS_Comeau 1

#elif defined(__INTEL_COMPILER) || defined(__ICC)
	#undef PORTABILITY_COMPILER_IS_Intel
	#define PORTABILITY_COMPILER_IS_Intel 1

#elif defined(__PATHCC__)
	#undef PORTABILITY_COMPILER_IS_PathScale
	#define PORTABILITY_COMPILER_IS_PathScale 1

#elif defined(__BORLANDC__) && defined(__CODEGEARC_VERSION__)
	#undef PORTABILITY_COMPILER_IS_Embarcadero
	#define PORTABILITY_COMPILER_IS_Embarcadero 1

#elif defined(__BORLANDC__)
	#undef PORTABILITY_COMPILER_IS_Borland
	#define PORTABILITY_COMPILER_IS_Borland 1

#elif defined(__WATCOMC__) && __WATCOMC__ < 1200
	#undef PORTABILITY_COMPILER_IS_Watcom
	#define PORTABILITY_COMPILER_IS_Watcom 1

#elif defined(__WATCOMC__)
	#undef PORTABILITY_COMPILER_IS_OpenWatcom
	#define PORTABILITY_COMPILER_IS_OpenWatcom 1

#elif defined(__SUNPRO_CC)
	#undef PORTABILITY_COMPILER_IS_SunPro
	#define PORTABILITY_COMPILER_IS_SunPro 1

#elif defined(__HP_aCC)
	#undef PORTABILITY_COMPILER_IS_HP
	#define PORTABILITY_COMPILER_IS_HP 1

#elif defined(__DECCXX)
	#undef PORTABILITY_COMPILER_IS_Compaq
	#define PORTABILITY_COMPILER_IS_Compaq 1

#elif defined(__IBMCPP__) && defined(__COMPILER_VER__)
	#undef PORTABILITY_COMPILER_IS_zOS
	#define PORTABILITY_COMPILER_IS_zOS 1

#elif defined(__ibmxl__) && defined(__clang__)
	#undef PORTABILITY_COMPILER_IS_XLClang
	#define PORTABILITY_COMPILER_IS_XLClang 1

#elif defined(__IBMCPP__) && !defined(__COMPILER_VER__) && __IBMCPP__ >= 800
	#undef PORTABILITY_COMPILER_IS_XL
	#define PORTABILITY_COMPILER_IS_XL 1

#elif defined(__IBMCPP__) && !defined(__COMPILER_VER__) && __IBMCPP__ < 800
	#undef PORTABILITY_COMPILER_IS_VisualAge
	#define PORTABILITY_COMPILER_IS_VisualAge 1

#elif defined(__PGI)
	#undef PORTABILITY_COMPILER_IS_PGI
	#define PORTABILITY_COMPILER_IS_PGI 1

#elif defined(_CRAYC)
	#undef PORTABILITY_COMPILER_IS_Cray
	#define PORTABILITY_COMPILER_IS_Cray 1

#elif defined(__TI_COMPILER_VERSION__)
	#undef PORTABILITY_COMPILER_IS_TI
	#define PORTABILITY_COMPILER_IS_TI 1

#elif defined(__FUJITSU) || defined(__FCC_VERSION) || defined(__fcc_version)
	#undef PORTABILITY_COMPILER_IS_Fujitsu
	#define PORTABILITY_COMPILER_IS_Fujitsu 1

#elif defined(__ghs__)
	#undef PORTABILITY_COMPILER_IS_GHS
	#define PORTABILITY_COMPILER_IS_GHS 1

#elif defined(__SCO_VERSION__)
	#undef PORTABILITY_COMPILER_IS_SCO
	#define PORTABILITY_COMPILER_IS_SCO 1

#elif defined(__ARMCC_VERSION) && !defined(__clang__)
	#undef PORTABILITY_COMPILER_IS_ARMCC
	#define PORTABILITY_COMPILER_IS_ARMCC 1

#elif defined(__clang__) && defined(__apple_build_version__)
	#undef PORTABILITY_COMPILER_IS_AppleClang
	#define PORTABILITY_COMPILER_IS_AppleClang 1

#elif defined(__clang__) && defined(__ARMCOMPILER_VERSION)
	#undef PORTABILITY_COMPILER_IS_ARMClang
	#define PORTABILITY_COMPILER_IS_ARMClang 1

#elif defined(__clang__)
	#undef PORTABILITY_COMPILER_IS_Clang
	#define PORTABILITY_COMPILER_IS_Clang 1

#elif defined(__GNUC__) || defined(__GNUG__)
	#undef PORTABILITY_COMPILER_IS_GNU
	#define PORTABILITY_COMPILER_IS_GNU 1

#elif defined(_MSC_VER)
	#undef PORTABILITY_COMPILER_IS_MSVC
	#define PORTABILITY_COMPILER_IS_MSVC 1

#elif defined(__VISUALDSPVERSION__) || defined(__ADSPBLACKFIN__) || defined(__ADSPTS__) || defined(__ADSP21000__)
	#undef PORTABILITY_COMPILER_IS_ADSP
	#define PORTABILITY_COMPILER_IS_ADSP 1

#elif defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ICC)
	#undef PORTABILITY_COMPILER_IS_IAR
	#define PORTABILITY_COMPILER_IS_IAR 1

#endif

#if PORTABILITY_COMPILER_IS_AppleClang

	#if !(((__clang_major__ * 100) + __clang_minor__) >= 400)
		#error "Unsupported compiler version"
	#endif

	#define PORTABILITY_COMPILER_VERSION_MAJOR (__clang_major__)
	#define PORTABILITY_COMPILER_VERSION_MINOR (__clang_minor__)
	#define PORTABILITY_COMPILER_VERSION_PATCH (__clang_patchlevel__)
	#if defined(_MSC_VER)
		/* _MSC_VER = VVRR */
		#define PORTABILITY_SIMULATE_VERSION_MAJOR (_MSC_VER / 100)
		#define PORTABILITY_SIMULATE_VERSION_MINOR (_MSC_VER % 100)
	#endif
	#define PORTABILITY_COMPILER_VERSION_TWEAK (__apple_build_version__)

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_alignas)
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_alignas)
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_constexpr)
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 1
	#else
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_override_control)
		#define PORTABILITY_COMPILER_CXX_FINAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_FINAL 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_noexcept)
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 1
	#else
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_nullptr)
		#define PORTABILITY_COMPILER_CXX_NULLPTR 1
	#else
		#define PORTABILITY_COMPILER_CXX_NULLPTR 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 1
	#else
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_thread_local)
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 0
	#endif

#elif PORTABILITY_COMPILER_IS_Clang

	#if !(((__clang_major__ * 100) + __clang_minor__) >= 301)
		#error "Unsupported compiler version"
	#endif

	#define PORTABILITY_COMPILER_VERSION_MAJOR (__clang_major__)
	#define PORTABILITY_COMPILER_VERSION_MINOR (__clang_minor__)
	#define PORTABILITY_COMPILER_VERSION_PATCH (__clang_patchlevel__)
	#if defined(_MSC_VER)
		/* _MSC_VER = VVRR */
		#define PORTABILITY_SIMULATE_VERSION_MAJOR (_MSC_VER / 100)
		#define PORTABILITY_SIMULATE_VERSION_MINOR (_MSC_VER % 100)
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_alignas)
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_alignas)
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_constexpr)
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 1
	#else
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_override_control)
		#define PORTABILITY_COMPILER_CXX_FINAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_FINAL 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_noexcept)
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 1
	#else
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_nullptr)
		#define PORTABILITY_COMPILER_CXX_NULLPTR 1
	#else
		#define PORTABILITY_COMPILER_CXX_NULLPTR 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 1
	#else
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 0
	#endif

	#if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_thread_local)
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 0
	#endif

#elif PORTABILITY_COMPILER_IS_GNU

	#if !((__GNUC__ * 100 + __GNUC_MINOR__) >= 404)
		#error "Unsupported compiler version"
	#endif

	#if defined(__GNUC__)
		#define PORTABILITY_COMPILER_VERSION_MAJOR (__GNUC__)
	#else
		#define PORTABILITY_COMPILER_VERSION_MAJOR (__GNUG__)
	#endif
	#if defined(__GNUC_MINOR__)
		#define PORTABILITY_COMPILER_VERSION_MINOR (__GNUC_MINOR__)
	#endif
	#if defined(__GNUC_PATCHLEVEL__)
		#define PORTABILITY_COMPILER_VERSION_PATCH (__GNUC_PATCHLEVEL__)
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 1
	#else
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_FINAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_FINAL 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 1
	#else
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
		#define PORTABILITY_COMPILER_CXX_NULLPTR 1
	#else
		#define PORTABILITY_COMPILER_CXX_NULLPTR 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 404 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 1
	#else
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 0
	#endif

	#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 && __cplusplus >= 201103L
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 0
	#endif

#elif PORTABILITY_COMPILER_IS_MSVC

	#if !(_MSC_VER >= 1600)
		#error "Unsupported compiler version"
	#endif

	/* _MSC_VER = VVRR */
	#define PORTABILITY_COMPILER_VERSION_MAJOR (_MSC_VER / 100)
	#define PORTABILITY_COMPILER_VERSION_MINOR (_MSC_VER % 100)
	#if defined(_MSC_FULL_VER)
		#if _MSC_VER >= 1400
			/* _MSC_FULL_VER = VVRRPPPPP */
			#define PORTABILITY_COMPILER_VERSION_PATCH (_MSC_FULL_VER % 100000)
		#else
			/* _MSC_FULL_VER = VVRRPPPP */
			#define PORTABILITY_COMPILER_VERSION_PATCH (_MSC_FULL_VER % 10000)
		#endif
	#endif
	#if defined(_MSC_BUILD)
		#define PORTABILITY_COMPILER_VERSION_TWEAK (_MSC_BUILD)
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNAS 0
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 1
	#else
		#define PORTABILITY_COMPILER_CXX_ALIGNOF 0
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 1
	#else
		#define PORTABILITY_COMPILER_CXX_CONSTEXPR 0
	#endif

	#if _MSC_VER >= 1700
		#define PORTABILITY_COMPILER_CXX_FINAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_FINAL 0
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 1
	#else
		#define PORTABILITY_COMPILER_CXX_NOEXCEPT 0
	#endif

	#if _MSC_VER >= 1600
		#define PORTABILITY_COMPILER_CXX_NULLPTR 1
	#else
		#define PORTABILITY_COMPILER_CXX_NULLPTR 0
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 1
	#else
		#define PORTABILITY_COMPILER_CXX_SIZEOF_MEMBER 0
	#endif

	#if _MSC_VER >= 1900
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 1
	#else
		#define PORTABILITY_COMPILER_CXX_THREAD_LOCAL 0
	#endif

#else
	#error "Unsupported compiler"
#endif

#if defined(PORTABILITY_COMPILER_CXX_ALIGNAS) && PORTABILITY_COMPILER_CXX_ALIGNAS
	#define PORTABILITY_ALIGNAS(X) alignas(X)
#elif PORTABILITY_COMPILER_IS_GNU || PORTABILITY_COMPILER_IS_Clang || PORTABILITY_COMPILER_IS_AppleClang
	#define PORTABILITY_ALIGNAS(X) __attribute__((__aligned__(X)))
#elif PORTABILITY_COMPILER_IS_MSVC
	#define PORTABILITY_ALIGNAS(X) __declspec(align(X))
#else
	#define PORTABILITY_ALIGNAS(X)
#endif

#if defined(PORTABILITY_COMPILER_CXX_ALIGNOF) && PORTABILITY_COMPILER_CXX_ALIGNOF
	#define PORTABILITY_ALIGNOF(X) alignof(X)
#elif PORTABILITY_COMPILER_IS_GNU || PORTABILITY_COMPILER_IS_Clang || PORTABILITY_COMPILER_IS_AppleClang
	#define PORTABILITY_ALIGNOF(X) __alignof__(X)
#elif PORTABILITY_COMPILER_IS_MSVC
	#define PORTABILITY_ALIGNOF(X) __alignof(X)
#endif

#if defined(PORTABILITY_COMPILER_CXX_CONSTEXPR) && PORTABILITY_COMPILER_CXX_CONSTEXPR
	#define PORTABILITY_CONSTEXPR constexpr
#else
	#define PORTABILITY_CONSTEXPR
#endif

#if defined(PORTABILITY_COMPILER_CXX_FINAL) && PORTABILITY_COMPILER_CXX_FINAL
	#define PORTABILITY_FINAL final
#else
	#define PORTABILITY_FINAL
#endif

#if defined(PORTABILITY_COMPILER_CXX_NOEXCEPT) && PORTABILITY_COMPILER_CXX_NOEXCEPT
	#define PORTABILITY_NOEXCEPT		 noexcept
	#define PORTABILITY_NOEXCEPT_EXPR(X) noexcept(X)
#else
	#define PORTABILITY_NOEXCEPT
	#define PORTABILITY_NOEXCEPT_EXPR(X)
#endif

#if defined(PORTABILITY_COMPILER_CXX_NULLPTR) && PORTABILITY_COMPILER_CXX_NULLPTR
	#define PORTABILITY_NULLPTR nullptr
#elif PORTABILITY_COMPILER_IS_GNU
	#define PORTABILITY_NULLPTR __null
#else
	#define PORTABILITY_NULLPTR 0
#endif

#if defined(PORTABILITY_COMPILER_CXX_THREAD_LOCAL) && PORTABILITY_COMPILER_CXX_THREAD_LOCAL
	#define PORTABILITY_THREAD_LOCAL thread_local
#elif PORTABILITY_COMPILER_IS_GNU || PORTABILITY_COMPILER_IS_Clang || PORTABILITY_COMPILER_IS_AppleClang
	#define PORTABILITY_THREAD_LOCAL __thread
#elif PORTABILITY_COMPILER_IS_MSVC
	#define PORTABILITY_THREAD_LOCAL __declspec(thread)
#else
// PORTABILITY_THREAD_LOCAL not defined for this configuration.
#endif

#endif /* PORTABILITY_COMPILER_DETECTION_H */
