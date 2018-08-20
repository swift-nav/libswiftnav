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

#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include <swiftnav/logging.h>

/** \defgroup logging Logging
 * Logging functions.
 * \{ */

/** Log message by level.
 *
 * \param level Log level
 * \param msg Log contents
 */
void log_(u8 level, const char *msg, ...) {
  va_list ap;

  fprintf(stderr, "%s: ", level_string[level]);
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

/** Log message by level with file path and line number.
 *
 * \param level Log level
 * \param file_path string of full path to file where this function was called
 * \param line_number line number where this function was called
 * \param msg Log contents
 */
void detailed_log_(u8 level,
                   const char *file_path,
                   const int line_number,
                   const char *msg,
                   ...) {
  va_list ap;
  fprintf(stderr, "(lsn::%s:%d) ", file_path, line_number);
  fprintf(stderr, "%s: ", level_string[level]);
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

/** Log an event.
 *
 *  By default, this will log to stderr on x86, but is intended for logging to a
 *  file on Piksi.
 *
 * \param fd Bogus file descriptor
 * \param msg Log string contents
 */
void event_(int fd, const char *msg, ...) {
  va_list ap;
  fprintf(stderr, "fd: %d: ", fd);
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
}

/* \} */
