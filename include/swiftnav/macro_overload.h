/*
 * Copyright (C) 2015 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef LIBSWIFTNAV_MACRO_OVERLOAD_H
#define LIBSWIFTNAV_MACRO_OVERLOAD_H

/**
 * Helper for overloaded macros. Define an overloaded macro by passing in
 * __VA_ARGS__ followed by a list of macros which accept a different number of
 * parameters in decending order, such as:
 *
 * #define FOO0() ...
 * #define FOO1(a) ...
 * #define FOO2(a,b) ...
 * #define FOO3(a,b,c) ...
 * #define FOO LSN_EXPAND(LSN_GET_MACRO(__VA_ARGS, FOO3, FOO2, FOO1, \
 * FOO0)(__VA_ARGS__))
 *
 * Then all the following calls are valid and get redirected to the correct
 * macro
 *
 * FOO()
 * FOO(one)
 * FOO(one,two)
 * FOO(one,two,three)
 *
 * This currently only works up to 16 arguments but can be easily expanded if
 * and when required
 */
#define LSN_GET_MACRO(_1,   \
                      _2,   \
                      _3,   \
                      _4,   \
                      _5,   \
                      _6,   \
                      _7,   \
                      _8,   \
                      _9,   \
                      _10,  \
                      _11,  \
                      _12,  \
                      _13,  \
                      _14,  \
                      _15,  \
                      _16,  \
                      NAME, \
                      ...)  \
  NAME

/**
 * Helper macro to get around MSVC preprocessor
 */
#define LSN_EXPAND(x) x

#endif
