/*
 * Copyright (C) 2021 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef SWIFTNAV_MACROS_H
#define SWIFTNAV_MACROS_H

/**
 * Compiler agnostic macros
 *
 * Every supported compiler needs to have a section added to this file to
 * override the list of macros. Macros defined in this file all replace some
 * sort of common annotation task (pragma, attributes, annotations etc) in a way
 * which allows other code to be written in a far more portable way.
 *
 * Available macros:
 *
 * SWIFT_PACK_START, SWIFT_PACK_END, SWIFT_ATTR_PACKED
 * Controls struct packing. In recognition that different compilers have
 * different packing semantics code should be written as follows:
 * @code
 * SWIFT_PACK_START
 * struct type_which_needs_to_be_packed SWIFT_ATTR_PACKED {
 *   int a;
 *   int b;
 * };
 * SWIFT_PACK_END
 * @endcode
 *
 * SWIFT_MESSAGE
 * Generates an information message (not a warning or error) at compile time.
 * @code
 * SWIFT_MESSAGE("compile time message")
 * @endcode
 *
 * SWIFT_ATTR_DEPRECATED
 * Mark a symbol as deprecated. Will cause a compiler warning for each use of
 * said symbol
 * @code
 * struct foo SWIFT_ATTR_DEPRECATED {...};
 * SWIFT_ATTR_DEPRECATED void bar(void);
 * @endcode
 *
 * SWIFT_ATTR_FORMAT(fmt,args), SWIFT_ATTR_VFORMAT(fmt),
 * SWIFT_ATTR_FORMAT_STRING printf format checking for functions. Use
 * SWIFT_ATTR_FORMAT for printf style and SWIFT_ATTR_VFORMAT for vprintf style,
 * apply SWIFT_ATTR_FORMAT_STRING directly to the format string argument. eg:
 * @code
 * SWIFT_ATTR_FORMAT(1,2) void log(const char *fmt SWIFT_ATTR_FORMAT_STRING,
 *                                 ...);
 * SWIFT_ATTR_VFORMAT(1) void vlog(const char *fmt SWIFT_ATTR_FORMAT_STRING,
 *                                 va_list ap);
 * @endcode
 *
 * SWIFT_ATTR_UNUSED
 * Mark a parameter as potentially unused.
 * @code
 * void foo(int bar SWIFT_ATTR_UNUSED);
 * @endcode
 *
 * SWIFT_ATTR_ALIGNED(n)
 * Align a type, member, or variable to a particular boundary
 * @code
 * SWIFT_ATTR_ALIGNED(8) int foo;
 * struct bar {
 *   SWIFT_ATTR_ALIGNED(4) int a;
 * };
 * @endcode
 *
 * SWIFT_ATTR_NORETURN
 * Mark a function as no return
 * @code
 * SWIFT_ATTR_NORETURN void run_forever(void);
 * @endcode
 *
 * Authors of this file do not need to add overloads for every macro when
 * supporting a new compiler. Only macros which actually affect the compiled
 * output are required, macros which simply provide information or optimisations
 * are not. The minimum list of macros which are required is:
 * SWIFT_PACK_START
 * SWIFT_PACK_END
 * SWIFT_ATTR_PACKED
 * SWIFT_ATTR_ALIGNED
 *
 * All other macros are considered optional, should a particular compiler not
 * provide implementation for optional macros they will be defined with default
 * values which have no effect.
 */

#if defined(__GNUC__) || defined(__clang__)
/*
 * GCC and clang are compatible with each other
 */

#define SWIFT_PACK_START /* Intentionally empty */
#define SWIFT_PACK_END   /* Intentionally empty */
#define SWIFT_ATTR_PACKED __attribute__((packed))
#define DO_PRAGMA(x) _Pragma(#x)
#define SWIFT_MESSAGE(msg) DO_PRAGMA(message(msg))
#define SWIFT_ATTR_DEPRECATED __attribute__((deprecated))
#define SWIFT_ATTR_FORMAT(fmt, args) __attribute__((format(printf, fmt, args)))
#define SWIFT_ATTR_VFORMAT(fmt) __attribute__((format(printf, fmt, 0)))
#define SWIFT_ATTR_UNUSED __attribute__((unused))
#define SWIFT_ATTR_ALIGNED(n) __attribute__((aligned(n)))
#define SWIFT_ATTR_NORETURN __attribute__((noreturn))
#define SWIFT_DECLSPEC __attribute__((visibility("default")))

// This attribute has sporadic compatibility with GCC, but since
// we always use clang to run UBSAN we can safely restrict it
// to just clang here
#if defined(__clang__)
#define SWIFT_ATTR_NO_SANITIZE_ENUM __attribute__((no_sanitize("enum")))
#endif

#elif defined(_MSC_VER)
/*
 * MSVC
 */

#include <sal.h>

#define SWIFT_PACK_START __pragma(pack(1));
#define SWIFT_PACK_END __pragma(pack());
#define SWIFT_ATTR_PACKED /* Intentionally empty */
#define SWIFT_ATTR_DEPRECATED __declspec(deprecated)
#define SWIFT_ATTR_ALIGNED(n) __declspec(align(n))
#if _MSC_VER >= 1400
#define SWIFT_ATTR_FORMAT_STRING _Printf_format_string_
#else
#define SWIFT_ATTR_FORMAT_STRING __format_string
#endif
#if !defined(_WINDLL)
#define SWIFT_DECLSPEC
#elif defined(swiftnav_EXPORTS)
#define SWIFT_DECLSPEC __declspec(dllexport)
#else
#define SWIFT_DECLSPEC __declspec(dllimport)
#endif

#elif defined(__ghs__)
/*
 * Greenhills
 */

#define SWIFT_PACK_START /* Intentionally empty */
#define SWIFT_PACK_END   /* Intentionally empty */
#define SWIFT_ATTR_PACKED __attribute__((packed))
#define SWIFT_ATTR_ALIGNED(n) __attribute__((aligned(n)))

#endif

/*
 * Check that the compiler definitions has defined all required macros
 */
#if !defined(SWIFT_PACK_START)
#error Please add a definition for SWIFT_PACK_START for the compiler in use
#endif
#if !defined(SWIFT_PACK_END)
#error Please add a definition for SWIFT_PACK_END for the compiler in use
#endif
#if !defined(SWIFT_ATTR_PACKED)
#error Please add a definition for SWIFT_ATTR_PACKED for the compiler in use
#endif
#if !defined(SWIFT_ATTR_ALIGNED)
#error Please add a definition for SWIFT_ATTR_ALIGNED for the compiler in use
#endif

/**
 * Now generate default implementations for all optional macros which weren't
 * defined for this compiler. The defaults implementations all have no effect
 */
#if !defined(SWIFT_MESSAGE)
#define SWIFT_MESSAGE(msg)
#endif
#if !defined(SWIFT_ATTR_DEPRECATED)
#define SWIFT_ATTR_DEPRECATED
#endif
#if !defined(SWIFT_ATTR_FORMAT)
#define SWIFT_ATTR_FORMAT(fmt, args)
#endif
#if !defined(SWIFT_ATTR_VFORMAT)
#define SWIFT_ATTR_VFORMAT(fmt)
#endif
#if !defined(SWIFT_ATTR_FORMAT_STRING)
#define SWIFT_ATTR_FORMAT_STRING
#endif
#if !defined(SWIFT_ATTR_UNUSED)
#define SWIFT_ATTR_UNUSED
#endif
#if !defined(SWIFT_ATTR_NORETURN)
#define SWIFT_ATTR_NORETURN
#endif
#if !defined(SWIFT_DECLSPEC)
#define SWIFT_DECLSPEC
#endif
#if !defined(SWIFT_ATTR_NO_SANITIZE_ENUM)
#define SWIFT_ATTR_NO_SANITIZE_ENUM
#endif

#endif
