/**
 * Copyright (C) 2021 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

/******************************************************************************
 * Automatically generated from scripts/leap_seconds_generator.py. Please do  *
 * not hand edit!                                                             *
 *                                                                            *
 * Updated: 14-07-2025                                                        *
 ******************************************************************************/

#ifndef LIBSWIFTNAV_LEAP_SECONDS_H
#define LIBSWIFTNAV_LEAP_SECONDS_H

#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start times of UTC leap second events given in GPS time {wn, tow, gps-utc}
 * The leap second event lasts for one second from the start time, and after
 * that the new offset is in effect.
 */
static const s32 utc_leaps[][3] = {
    {77, 259200, 1},    /* 01-07-1981 */
    {129, 345601, 2},   /* 01-07-1982 */
    {181, 432002, 3},   /* 01-07-1983 */
    {286, 86403, 4},    /* 01-07-1985 */
    {416, 432004, 5},   /* 01-01-1988 */
    {521, 86405, 6},    /* 01-01-1990 */
    {573, 172806, 7},   /* 01-01-1991 */
    {651, 259207, 8},   /* 01-07-1992 */
    {703, 345608, 9},   /* 01-07-1993 */
    {755, 432009, 10},  /* 01-07-1994 */
    {834, 86410, 11},   /* 01-01-1996 */
    {912, 172811, 12},  /* 01-07-1997 */
    {990, 432012, 13},  /* 01-01-1999 */
    {1356, 13, 14},     /* 01-01-2006 */
    {1512, 345614, 15}, /* 01-01-2009 */
    {1695, 15, 16},     /* 01-07-2012 */
    {1851, 259216, 17}, /* 01-07-2015 */
    {1930, 17, 18},     /* 01-01-2017 */
};

/** GPS time when the utc_leaps table expires 28-06-2026 */
static const s32 gps_time_utc_leaps_expiry[2] = {2425, 18};

/** UNIX time when the utc_leaps table expires 28-06-2026 */
static const s64 unix_time_utc_leaps_expiry = 1782604800;

#ifdef __cplusplus
}
#endif

#endif  // LIBSWIFTNAV_LEAP_SECONDS_H
