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

#ifdef LIBSWIFTNAV_ENABLE_STDERR_LOGGING

/** \defgroup logging Logging
 * Logging functions.
 * \{ */

/** Log message by level.
 *
 * \param level Log level
 * \param msg Log contents
 */
SWIFT_ATTR_FORMAT(2, 3)
static void log_stderr(int level,
                       SWIFT_ATTR_FORMAT_STRING const char *msg,
                       ...) {
  va_list ap;
  fprintf(stderr, "%s: ", level_string[level]);
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);  // NOLINT - clang-tidy insists that ap is
                              // uninitialised when it clearly is
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
SWIFT_ATTR_FORMAT(4, 5)
static void detailed_log_stderr(int level,
                                const char *file_path,
                                const int line_number,
                                SWIFT_ATTR_FORMAT_STRING const char *msg,
                                ...) {
  va_list ap;
  fprintf(stderr, "(lsn::%s:%d) ", file_path, line_number);
  fprintf(stderr, "%s: ", level_string[level]);
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);  // NOLINT - clang-tidy insists that ap is
                              // uninitialised when is clearly is
  va_end(ap);
  fprintf(stderr, "\n");
}

#define DEFAULT_LOG log_stderr
#define DEFAULT_DETAILED_LOG detailed_log_stderr

#else

#define DEFAULT_LOG NULL
#define DEFAULT_DETAILED_LOG NULL

#endif

pfn_log log_ = DEFAULT_LOG;
pfn_detailed_log detailed_log_ = DEFAULT_DETAILED_LOG;

void logging_set_implementation(pfn_log impl_log,
                                pfn_detailed_log impl_detailed_log) {
  if (impl_log == NULL && impl_detailed_log == NULL) {
    log_ = DEFAULT_LOG;
    detailed_log_ = DEFAULT_DETAILED_LOG;
    return;
  }

  assert(impl_log != NULL);
  assert(impl_detailed_log != NULL);
  log_ = impl_log;
  detailed_log_ = impl_detailed_log;
}

/* \} */
