#!/usr/bin/python3

import datetime
import jinja2
import sys
import urllib.request

DATE_TIME_FORMAT = "{day:02d}-{month:02d}-{year}"
GPS_EPOCH = datetime.datetime(1980, 1, 6)
TAI_GPS_OFFSET = 19
TAI_UNIX_OFFSET = 37
UNIX_EPOCH = datetime.datetime(1970, 1, 1)
UTC_EPOCH = datetime.datetime(1900, 1, 1)

JINJA_GNSS_TIME_LATEST_TEMPLATE = """/**
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
 * Updated: {{now.to_date_string()}}                                                        *
 ******************************************************************************/

#ifndef LIBSWIFTNAV_LEAP_SECONDS_H
#define LIBSWIFTNAV_LEAP_SECONDS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start times of UTC leap second events given in GPS time {wn, tow, gps-utc}
 * The leap second event lasts for one second from the start time, and after
 * that the new offset is in effect.
 */
static const s32 utc_leaps[][3] = {
{% for utc_leap_item in utc_leap_list %}
{%- if utc_leap_item.to_gps_clock().seconds_since_epoch() >= 0 -%}
  { {{utc_leap_item.to_gps_clock().offset_seconds(-1).wn()}}, {{utc_leap_item.to_gps_clock().offset_seconds(-1).tow()}}, {{utc_leap_item.tai_utc_offset() - TAI_GPS_OFFSET}} }, /* {{utc_leap_item.to_date_string()}} */
{% endif %}
{%- endfor -%}
};

/** GPS time when the utc_leaps table expires {{utc_leap_list_expires.to_date_string()}} */
static const s32 gps_time_utc_leaps_expiry[2] = { {{utc_leap_list_expires.to_gps_clock().wn()}}, {{utc_leap_list_expires.to_gps_clock().tow()}} };

/** UNIX time when the utc_leaps table expires {{utc_leap_list_expires.to_date_string()}} */
static const s64 unix_time_utc_leaps_expiry = {{utc_leap_list_expires.to_unix_clock().seconds_since_epoch()}};

#ifdef __cplusplus
}
#endif

#endif  // LIBSWIFTNAV_LEAP_SECONDS_H

"""


class UtcClock:
    def __init__(self, utc_time, tai_utc_offset):
        self._utc_time = utc_time
        self._tai_utc_offset = tai_utc_offset

    def utc_time(self):
        return self._utc_time

    def tai_utc_offset(self):
        return self._tai_utc_offset

    def to_gps_clock(self):
        return GpsClock(
            self._utc_time + self._tai_utc_offset - (GPS_EPOCH - UTC_EPOCH).total_seconds() - TAI_GPS_OFFSET)

    def to_unix_clock(self):
        return UnixClock(
            self._utc_time + self._tai_utc_offset - (UNIX_EPOCH - UTC_EPOCH).total_seconds() - TAI_UNIX_OFFSET)

    def to_date_string(self):
        date = UTC_EPOCH + datetime.timedelta(0, self._utc_time)
        return DATE_TIME_FORMAT.format(**{"day": date.day, "month": date.month, "year": date.year})


class GpsClock:
    def __init__(self, gps_time):
        self._gps_time = gps_time

    def wn(self):
        return int(self._gps_time / 604800)

    def tow(self):
        return int(self._gps_time % 604800)

    def seconds_since_epoch(self):
        return int(self._gps_time)

    def offset_seconds(self, seconds):
        return GpsClock(self._gps_time + seconds)


class UnixClock:
    def __init__(self, unix_time):
        self._unix_time = unix_time

    def seconds_since_epoch(self):
        return int(self._unix_time)

    def offset_seconds(self, seconds):
        return UnixClock(self._unix_time + seconds)


if len(sys.argv) != 2:
    print("error: usage <header file location>", file=sys.stderr)
    exit(1)

utc_leap_list = []
utc_leap_list_expires_utc_time = None

with urllib.request.urlopen("https://www.ietf.org/timezones/data/leap-seconds.list") as response:
    response_content = response.read()
    for line in response_content.decode("utf-8").splitlines():
        is_comment = line.startswith("#")
        is_comment_expiry_time = line.startswith("#@")

        if is_comment and not is_comment_expiry_time:
            continue

        entries = line.split()

        if is_comment_expiry_time:
            utc_leap_list_expires_utc_time = int(entries[1])
        else:
            utc_leap_list.append(UtcClock(int(entries[0]), int(entries[1])))

utc_leap_list_expires = UtcClock(utc_leap_list_expires_utc_time, utc_leap_list[-1].tai_utc_offset())
now = UtcClock((datetime.datetime.utcnow() - UTC_EPOCH).total_seconds(), utc_leap_list[-1].tai_utc_offset())

gnss_time_latest_template = jinja2.Template(JINJA_GNSS_TIME_LATEST_TEMPLATE)
gnss_time_latest_render = gnss_time_latest_template.render({
    "now": now,
    "utc_leap_list": utc_leap_list,
    "utc_leap_list_expires": utc_leap_list_expires,
    "TAI_GPS_OFFSET": TAI_GPS_OFFSET,
})

with open(sys.argv[1], "w") as file:
    file.write(gnss_time_latest_render)
