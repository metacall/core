/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef RB_LOADER_INCLUDE_H
#define RB_LOADER_INCLUDE_H 1

#if (defined(_WIN32) || defined(_WIN64)) && !defined(_MSC_VER) && defined(boolean)
	#undef boolean
#endif

/* Disable warnings from Ruby */
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wredundant-decls"
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <ruby.h>

/* Disable warnings from Ruby */
#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif

/* Backward compatible macros for Ruby < 2.7 */
#ifndef RB_PASS_KEYWORDS
	#define rb_funcallv_kw(o, m, c, v, kw)					 rb_funcallv(o, m, c, v)
	#define rb_funcallv_public_kw(o, m, c, v, kw)			 rb_funcallv_public(o, m, c, v)
	#define rb_funcall_passing_block_kw(o, m, c, v, kw)		 rb_funcall_passing_block(o, m, c, v)
	#define rb_funcall_with_block_kw(o, m, c, v, b, kw)		 rb_funcall_with_block(o, m, c, v, b)
	#define rb_scan_args_kw(kw, c, v, s, ...)				 rb_scan_args(c, v, s, __VA_ARGS__)
	#define rb_call_super_kw(c, v, kw)						 rb_call_super(c, v)
	#define rb_yield_values_kw(c, v, kw)					 rb_yield_values2(c, v)
	#define rb_yield_splat_kw(a, kw)						 rb_yield_splat(a)
	#define rb_block_call_kw(o, m, c, v, f, p, kw)			 rb_block_call(o, m, c, v, f, p)
	#define rb_fiber_resume_kw(o, c, v, kw)					 rb_fiber_resume(o, c, v)
	#define rb_fiber_yield_kw(c, v, kw)						 rb_fiber_yield(c, v)
	#define rb_enumeratorize_with_size_kw(o, m, c, v, f, kw) rb_enumeratorize_with_size(o, m, c, v, f)
	#define SIZED_ENUMERATOR_KW(obj, argc, argv, size_fn, kw_splat) \
		rb_enumeratorize_with_size((obj), ID2SYM(rb_frame_this_func()), \
			(argc), (argv), (size_fn))
	#define RETURN_SIZED_ENUMERATOR_KW(obj, argc, argv, size_fn, kw_splat) \
		do \
		{ \
			if (!rb_block_given_p()) \
				return SIZED_ENUMERATOR(obj, argc, argv, size_fn); \
		} while (0)
	#define RETURN_ENUMERATOR_KW(obj, argc, argv, kw_splat) RETURN_SIZED_ENUMERATOR(obj, argc, argv, 0)
	#define rb_check_funcall_kw(o, m, c, v, kw)				rb_check_funcall(o, m, c, v)
	#define rb_obj_call_init_kw(o, c, v, kw)				rb_obj_call_init(o, c, v)
	#define rb_class_new_instance_kw(c, v, k, kw)			rb_class_new_instance(c, v, k)
	#define rb_proc_call_kw(p, a, kw)						rb_proc_call(p, a)
	#define rb_proc_call_with_block_kw(p, c, v, b, kw)		rb_proc_call_with_block(p, c, v, b)
	#define rb_method_call_kw(c, v, m, kw)					rb_method_call(c, v, m)
	#define rb_method_call_with_block_kw(c, v, m, b, kw)	rb_method_call_with_block(c, v, m, b)
	#define rb_eval_cmd_kwd(c, a, kw)						rb_eval_cmd(c, a, 0)
#endif

#endif /* RB_LOADER_INCLUDE_H */
