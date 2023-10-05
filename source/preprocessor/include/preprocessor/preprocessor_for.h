/*
 *	Preprocessor Library by Parra Studios
 *	A generic header-only preprocessor metaprogramming library.
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

#ifndef PREPROCESSOR_FOR_H
#define PREPROCESSOR_FOR_H 1

/* -- Headers -- */

#include <preprocessor/preprocessor_api.h>

#include <preprocessor/preprocessor_arguments.h>
#include <preprocessor/preprocessor_tuple.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define PREPROCESSOR_FOR_SIZE 64

/* -- Macros -- */

/* Preprocessor foreach, it has a callback like: macro(value) for each element of the variadic arguments */
#define PREPROCESSOR_FOR_EACH_EVAL(expr) expr

#define PREPROCESSOR_FOR_EACH_IMPL_0(expr, ...)
#define PREPROCESSOR_FOR_EACH_IMPL_1(expr, element, ...) expr(element)
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
	(defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#	define PREPROCESSOR_FOR_EACH_IMPL_2(expr, element, ...) \
	expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_1(expr, __VA_ARGS__, ~))
#else
#	define PREPROCESSOR_FOR_EACH_IMPL_2(expr, element, ...) \
	expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_1(expr, __VA_ARGS__))
#endif
#define PREPROCESSOR_FOR_EACH_IMPL_3(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_2(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_4(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_3(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_5(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_4(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_6(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_5(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_7(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_6(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_8(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_7(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_9(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_8(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_10(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_9(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_11(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_10(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_12(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_11(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_13(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_12(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_14(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_13(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_15(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_14(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_16(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_15(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_17(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_16(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_18(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_17(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_19(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_18(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_20(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_19(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_21(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_20(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_22(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_21(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_23(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_22(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_24(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_23(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_25(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_24(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_26(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_25(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_27(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_26(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_28(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_27(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_29(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_28(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_30(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_29(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_31(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_30(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_32(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_31(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_33(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_32(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_34(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_33(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_35(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_34(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_36(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_35(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_37(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_36(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_38(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_37(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_39(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_38(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_40(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_39(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_41(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_40(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_42(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_41(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_43(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_42(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_44(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_43(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_45(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_44(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_46(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_45(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_47(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_46(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_48(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_47(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_49(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_48(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_50(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_49(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_51(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_50(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_52(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_51(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_53(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_52(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_54(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_53(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_55(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_54(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_56(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_55(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_57(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_56(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_58(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_57(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_59(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_58(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_60(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_59(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_61(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_60(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_62(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_61(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_63(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_62(expr, __VA_ARGS__))
#define PREPROCESSOR_FOR_EACH_IMPL_64(expr, element, ...) expr(element) PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_63(expr, __VA_ARGS__))

#if defined(__GNUC__) || defined(__clang__)
#	define PREPROCESSOR_FOR_EACH(expr, ...) \
		PREPROCESSOR_ARGS_N_IMPL(__VA_ARGS__, \
		PREPROCESSOR_FOR_EACH_IMPL_63, PREPROCESSOR_FOR_EACH_IMPL_62, PREPROCESSOR_FOR_EACH_IMPL_61, PREPROCESSOR_FOR_EACH_IMPL_60, \
		PREPROCESSOR_FOR_EACH_IMPL_59, PREPROCESSOR_FOR_EACH_IMPL_58, PREPROCESSOR_FOR_EACH_IMPL_57, PREPROCESSOR_FOR_EACH_IMPL_56, PREPROCESSOR_FOR_EACH_IMPL_55, PREPROCESSOR_FOR_EACH_IMPL_54, PREPROCESSOR_FOR_EACH_IMPL_53, PREPROCESSOR_FOR_EACH_IMPL_52, PREPROCESSOR_FOR_EACH_IMPL_51, PREPROCESSOR_FOR_EACH_IMPL_50, \
		PREPROCESSOR_FOR_EACH_IMPL_49, PREPROCESSOR_FOR_EACH_IMPL_48, PREPROCESSOR_FOR_EACH_IMPL_47, PREPROCESSOR_FOR_EACH_IMPL_46, PREPROCESSOR_FOR_EACH_IMPL_45, PREPROCESSOR_FOR_EACH_IMPL_44, PREPROCESSOR_FOR_EACH_IMPL_43, PREPROCESSOR_FOR_EACH_IMPL_42, PREPROCESSOR_FOR_EACH_IMPL_41, PREPROCESSOR_FOR_EACH_IMPL_40, \
		PREPROCESSOR_FOR_EACH_IMPL_39, PREPROCESSOR_FOR_EACH_IMPL_38, PREPROCESSOR_FOR_EACH_IMPL_37, PREPROCESSOR_FOR_EACH_IMPL_36, PREPROCESSOR_FOR_EACH_IMPL_35, PREPROCESSOR_FOR_EACH_IMPL_34, PREPROCESSOR_FOR_EACH_IMPL_33, PREPROCESSOR_FOR_EACH_IMPL_32, PREPROCESSOR_FOR_EACH_IMPL_31, PREPROCESSOR_FOR_EACH_IMPL_30, \
		PREPROCESSOR_FOR_EACH_IMPL_29, PREPROCESSOR_FOR_EACH_IMPL_28, PREPROCESSOR_FOR_EACH_IMPL_27, PREPROCESSOR_FOR_EACH_IMPL_26, PREPROCESSOR_FOR_EACH_IMPL_25, PREPROCESSOR_FOR_EACH_IMPL_24, PREPROCESSOR_FOR_EACH_IMPL_23, PREPROCESSOR_FOR_EACH_IMPL_22, PREPROCESSOR_FOR_EACH_IMPL_21, PREPROCESSOR_FOR_EACH_IMPL_20, \
		PREPROCESSOR_FOR_EACH_IMPL_19, PREPROCESSOR_FOR_EACH_IMPL_18, PREPROCESSOR_FOR_EACH_IMPL_17, PREPROCESSOR_FOR_EACH_IMPL_16, PREPROCESSOR_FOR_EACH_IMPL_15, PREPROCESSOR_FOR_EACH_IMPL_14, PREPROCESSOR_FOR_EACH_IMPL_13, PREPROCESSOR_FOR_EACH_IMPL_12, PREPROCESSOR_FOR_EACH_IMPL_11, PREPROCESSOR_FOR_EACH_IMPL_10, \
		PREPROCESSOR_FOR_EACH_IMPL_9, PREPROCESSOR_FOR_EACH_IMPL_8, PREPROCESSOR_FOR_EACH_IMPL_7, PREPROCESSOR_FOR_EACH_IMPL_6, PREPROCESSOR_FOR_EACH_IMPL_5, PREPROCESSOR_FOR_EACH_IMPL_4, PREPROCESSOR_FOR_EACH_IMPL_3, PREPROCESSOR_FOR_EACH_IMPL_2, PREPROCESSOR_FOR_EACH_IMPL_1, \
		PREPROCESSOR_FOR_EACH_IMPL_0)(expr, __VA_ARGS__)
#elif defined(_MSC_VER) && !defined(__clang__)
#	define PREPROCESSOR_FOR_EACH_IMPL_COUNT(count) \
		PREPROCESSOR_CONCAT(PREPROCESSOR_FOR_EACH_IMPL_, count)
#	define PREPROCESSOR_FOR_EACH_IMPL_EXPR(...) \
		PREPROCESSOR_FOR_EACH_IMPL_COUNT(PREPROCESSOR_ARGS_COUNT(__VA_ARGS__))
#	define PREPROCESSOR_FOR_EACH_EXPAND(expr, args) expr ## args
#	define PREPROCESSOR_FOR_EACH(expr, ...) \
		PREPROCESSOR_FOR_EACH_EXPAND(PREPROCESSOR_FOR_EACH_IMPL_EXPR(__VA_ARGS__), (expr, __VA_ARGS__))
#else
#	error "Unkwnown preprocessor implementation for foreach."
#endif

/*
 * Preprocessor for, it has a callback like: macro(context, iterator, element) for each element of the variadic arguments,
 * where context is the second value passed to PREPROCESSOR_FOR, iterator is an integer indicating the current iteration,
 * and element is the current element being iterated
 */
#define PREPROCESSOR_FOR_EVAL(expr) expr

#define PREPROCESSOR_FOR_IMPL_0(expr, context, ...)
#define PREPROCESSOR_FOR_IMPL_1(expr, context, element, ...) expr(context, 0, element)
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
	(defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#	define PREPROCESSOR_FOR_IMPL_2(expr, context, element, ...) \
	expr(context, 1, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_1(expr, context, __VA_ARGS__, ~))
#else
#	define PREPROCESSOR_FOR_IMPL_2(expr, context, element, ...) \
	expr(context, 1, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_1(expr, context, __VA_ARGS__))
#endif
#define PREPROCESSOR_FOR_IMPL_3(expr, context, element, ...) expr(context, 2, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_2(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_4(expr, context, element, ...) expr(context, 3, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_3(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_5(expr, context, element, ...) expr(context, 4, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_4(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_6(expr, context, element, ...) expr(context, 5, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_5(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_7(expr, context, element, ...) expr(context, 6, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_6(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_8(expr, context, element, ...) expr(context, 7, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_7(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_9(expr, context, element, ...) expr(context, 8, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_8(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_10(expr, context, element, ...) expr(context, 9, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_9(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_11(expr, context, element, ...) expr(context, 10, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_10(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_12(expr, context, element, ...) expr(context, 11, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_11(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_13(expr, context, element, ...) expr(context, 12, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_12(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_14(expr, context, element, ...) expr(context, 13, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_13(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_15(expr, context, element, ...) expr(context, 14, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_14(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_16(expr, context, element, ...) expr(context, 15, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_15(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_17(expr, context, element, ...) expr(context, 16, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_16(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_18(expr, context, element, ...) expr(context, 17, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_17(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_19(expr, context, element, ...) expr(context, 18, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_18(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_20(expr, context, element, ...) expr(context, 19, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_19(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_21(expr, context, element, ...) expr(context, 20, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_20(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_22(expr, context, element, ...) expr(context, 21, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_21(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_23(expr, context, element, ...) expr(context, 22, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_22(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_24(expr, context, element, ...) expr(context, 23, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_23(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_25(expr, context, element, ...) expr(context, 24, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_24(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_26(expr, context, element, ...) expr(context, 25, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_25(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_27(expr, context, element, ...) expr(context, 26, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_26(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_28(expr, context, element, ...) expr(context, 27, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_27(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_29(expr, context, element, ...) expr(context, 28, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_28(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_30(expr, context, element, ...) expr(context, 29, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_29(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_31(expr, context, element, ...) expr(context, 30, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_30(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_32(expr, context, element, ...) expr(context, 31, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_31(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_33(expr, context, element, ...) expr(context, 32, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_32(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_34(expr, context, element, ...) expr(context, 33, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_33(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_35(expr, context, element, ...) expr(context, 34, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_34(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_36(expr, context, element, ...) expr(context, 35, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_35(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_37(expr, context, element, ...) expr(context, 36, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_36(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_38(expr, context, element, ...) expr(context, 37, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_37(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_39(expr, context, element, ...) expr(context, 38, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_38(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_40(expr, context, element, ...) expr(context, 39, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_39(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_41(expr, context, element, ...) expr(context, 40, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_40(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_42(expr, context, element, ...) expr(context, 41, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_41(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_43(expr, context, element, ...) expr(context, 42, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_42(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_44(expr, context, element, ...) expr(context, 43, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_43(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_45(expr, context, element, ...) expr(context, 44, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_44(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_46(expr, context, element, ...) expr(context, 45, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_45(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_47(expr, context, element, ...) expr(context, 46, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_46(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_48(expr, context, element, ...) expr(context, 47, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_47(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_49(expr, context, element, ...) expr(context, 48, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_48(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_50(expr, context, element, ...) expr(context, 49, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_49(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_51(expr, context, element, ...) expr(context, 50, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_50(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_52(expr, context, element, ...) expr(context, 51, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_51(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_53(expr, context, element, ...) expr(context, 52, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_52(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_54(expr, context, element, ...) expr(context, 53, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_53(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_55(expr, context, element, ...) expr(context, 54, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_54(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_56(expr, context, element, ...) expr(context, 55, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_55(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_57(expr, context, element, ...) expr(context, 56, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_56(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_58(expr, context, element, ...) expr(context, 57, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_57(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_59(expr, context, element, ...) expr(context, 58, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_58(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_60(expr, context, element, ...) expr(context, 59, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_59(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_61(expr, context, element, ...) expr(context, 60, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_60(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_62(expr, context, element, ...) expr(context, 61, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_61(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_63(expr, context, element, ...) expr(context, 62, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_62(expr, context, __VA_ARGS__))
#define PREPROCESSOR_FOR_IMPL_64(expr, context, element, ...) expr(context, 63, element) PREPROCESSOR_FOR_EVAL(PREPROCESSOR_FOR_IMPL_63(expr, context, __VA_ARGS__))

#if defined(__GNUC__) || defined(__clang__)
#	define PREPROCESSOR_FOR(expr, context, ...) \
		PREPROCESSOR_ARGS_N_IMPL(__VA_ARGS__, \
		PREPROCESSOR_FOR_IMPL_63, PREPROCESSOR_FOR_IMPL_62, PREPROCESSOR_FOR_IMPL_61, PREPROCESSOR_FOR_IMPL_60, \
		PREPROCESSOR_FOR_IMPL_59, PREPROCESSOR_FOR_IMPL_58, PREPROCESSOR_FOR_IMPL_57, PREPROCESSOR_FOR_IMPL_56, PREPROCESSOR_FOR_IMPL_55, PREPROCESSOR_FOR_IMPL_54, PREPROCESSOR_FOR_IMPL_53, PREPROCESSOR_FOR_IMPL_52, PREPROCESSOR_FOR_IMPL_51, PREPROCESSOR_FOR_IMPL_50, \
		PREPROCESSOR_FOR_IMPL_49, PREPROCESSOR_FOR_IMPL_48, PREPROCESSOR_FOR_IMPL_47, PREPROCESSOR_FOR_IMPL_46, PREPROCESSOR_FOR_IMPL_45, PREPROCESSOR_FOR_IMPL_44, PREPROCESSOR_FOR_IMPL_43, PREPROCESSOR_FOR_IMPL_42, PREPROCESSOR_FOR_IMPL_41, PREPROCESSOR_FOR_IMPL_40, \
		PREPROCESSOR_FOR_IMPL_39, PREPROCESSOR_FOR_IMPL_38, PREPROCESSOR_FOR_IMPL_37, PREPROCESSOR_FOR_IMPL_36, PREPROCESSOR_FOR_IMPL_35, PREPROCESSOR_FOR_IMPL_34, PREPROCESSOR_FOR_IMPL_33, PREPROCESSOR_FOR_IMPL_32, PREPROCESSOR_FOR_IMPL_31, PREPROCESSOR_FOR_IMPL_30, \
		PREPROCESSOR_FOR_IMPL_29, PREPROCESSOR_FOR_IMPL_28, PREPROCESSOR_FOR_IMPL_27, PREPROCESSOR_FOR_IMPL_26, PREPROCESSOR_FOR_IMPL_25, PREPROCESSOR_FOR_IMPL_24, PREPROCESSOR_FOR_IMPL_23, PREPROCESSOR_FOR_IMPL_22, PREPROCESSOR_FOR_IMPL_21, PREPROCESSOR_FOR_IMPL_20, \
		PREPROCESSOR_FOR_IMPL_19, PREPROCESSOR_FOR_IMPL_18, PREPROCESSOR_FOR_IMPL_17, PREPROCESSOR_FOR_IMPL_16, PREPROCESSOR_FOR_IMPL_15, PREPROCESSOR_FOR_IMPL_14, PREPROCESSOR_FOR_IMPL_13, PREPROCESSOR_FOR_IMPL_12, PREPROCESSOR_FOR_IMPL_11, PREPROCESSOR_FOR_IMPL_10, \
		PREPROCESSOR_FOR_IMPL_9, PREPROCESSOR_FOR_IMPL_8, PREPROCESSOR_FOR_IMPL_7, PREPROCESSOR_FOR_IMPL_6, PREPROCESSOR_FOR_IMPL_5, PREPROCESSOR_FOR_IMPL_4, PREPROCESSOR_FOR_IMPL_3, PREPROCESSOR_FOR_IMPL_2, PREPROCESSOR_FOR_IMPL_1, \
		PREPROCESSOR_FOR_IMPL_0)(expr, context, __VA_ARGS__)
#elif defined(_MSC_VER) && !defined(__clang__)
#	define PREPROCESSOR_FOR_IMPL_COUNT(count) \
		PREPROCESSOR_CONCAT(PREPROCESSOR_FOR_IMPL_, count)
#	define PREPROCESSOR_FOR_IMPL_EXPR(...) \
		PREPROCESSOR_FOR_IMPL_COUNT(PREPROCESSOR_ARGS_COUNT(__VA_ARGS__))
#	define PREPROCESSOR_FOR_EXPAND(expr, args) expr ## args
#	define PREPROCESSOR_FOR(expr, context, ...) \
		PREPROCESSOR_FOR_EXPAND(PREPROCESSOR_FOR_IMPL_EXPR(__VA_ARGS__), (expr, context, __VA_ARGS__))
#else
#	error "Unkwnown preprocessor implementation for for."
#endif

#ifdef __cplusplus
}
#endif

#endif /* PREPROCESSOR_FOR_H */
