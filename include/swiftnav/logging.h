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

#ifndef LIBSWIFTNAV_LOGGING_H
#define LIBSWIFTNAV_LOGGING_H

#include <stdbool.h>
#include <stdio.h>

#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* DEBUG off by default, enable it on a per-file basis. */
#ifndef DEBUG
#define DEBUG false
#endif

/* LOG_LEVEL set to LOG_INFO by default. */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

/** \defgroup logging Logging
 * Logging
 *
 * Logging at the `DEBUG` level is turned off by default and should be enabled
 * on a per-file basis by adding the following line to the source file *before*
 * including `logging.h`:
 *
 *    \#define DEBUG 1
 *
 * \{ */

void log_(u8 level, const char *msg, ...) __attribute__((format(printf, 2, 3)));

void detailed_log_(u8 level,
                   const char *file_path,
                   const int line_number,
                   const char *msg,
                   ...) __attribute__((format(printf, 4, 5)));

const char *truncate_path_(char *path);

void event_(int fd, const char *msg, ...) __attribute__((format(printf, 2, 3)));

extern const char *level_string[];

#define LOG_EMERG 0  /* system is unusable */
#define LOG_ALERT 1  /* action must be taken immediately */
#define LOG_CRIT 2   /* critical conditions */
#define LOG_ERROR 3  /* error conditions */
#define LOG_WARN 4   /* warning conditions */
#define LOG_NOTICE 5 /* normal but significant condition */
#define LOG_INFO 6   /* informational */
#define LOG_DEBUG 7  /* debug-level messages */

/** Log an emergency.
 * \param args `printf` style format and arguments.
 */
#define log_emerg(args...)        \
  do {                            \
    if (LOG_LEVEL >= LOG_EMERG) { \
      log_(LOG_EMERG, args);      \
    }                             \
  } while (false)

/** Log an alert.
 * \param args `printf` style format and arguments.
 */
#define log_alert(args...)        \
  do {                            \
    if (LOG_LEVEL >= LOG_ALERT) { \
      log_(LOG_ALERT, args);      \
    }                             \
  } while (false)

/** Log a critical event.
 * \param args `printf` style format and arguments.
 */
#define log_crit(args...)        \
  do {                           \
    if (LOG_LEVEL >= LOG_CRIT) { \
      log_(LOG_CRIT, args);      \
    }                            \
  } while (false)

/** Log an error.
 * \param args `printf` style format and arguments.
 */
#define log_error(args...)        \
  do {                            \
    if (LOG_LEVEL >= LOG_ERROR) { \
      log_(LOG_ERROR, args);      \
    }                             \
  } while (false)

/** Log a warning.
 * \param args `printf` style format and arguments.
 */
#define log_warn(args...)        \
  do {                           \
    if (LOG_LEVEL >= LOG_WARN) { \
      log_(LOG_WARN, args);      \
    }                            \
  } while (false)

/** Log a notice.
 * \param args `printf` style format and arguments.
 */
#define log_notice(args...)        \
  do {                             \
    if (LOG_LEVEL >= LOG_NOTICE) { \
      log_(LOG_NOTICE, args);      \
    }                              \
  } while (false)

/** Log an information message.
 * \param args `printf` style format and arguments.
 */
#define log_info(args...)        \
  do {                           \
    if (LOG_LEVEL >= LOG_INFO) { \
      log_(LOG_INFO, args);      \
    }                            \
  } while (false)

/** Log a debugging message.
 * \param args `printf` style format and arguments.
 */
#define log_debug(args...)                 \
  do {                                     \
    if (DEBUG || LOG_LEVEL >= LOG_DEBUG) { \
      log_(LOG_DEBUG, args);               \
    }                                      \
  } while (false)

/** Truncates the path to the base file name before logging
 * \param log_level level of the logging (DEBUG, INFO, etc.)
 * \param full_path full path to the file where the logger was called
 * \param line      line number where the logger was called
 * \param args      `printf` style format and argumebts
 */
#define detailed_log_truncated_(log_level, full_path, line, args...) \
  do {                                                               \
    char path[255] = full_path;                                      \
    detailed_log_(log_level, truncate_path_(path), line, args);      \
  } while (false)

/** Log an emergency (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_emerg(args...)                                 \
  do {                                                              \
    if (LOG_LEVEL >= LOG_EMERG) {                                   \
      detailed_log_truncated_(LOG_EMERG, __FILE__, __LINE__, args); \
    }                                                               \
  } while (false)

/** Log an alert. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_alert(args...)                                 \
  do {                                                              \
    if (LOG_LEVEL >= LOG_ALERT) {                                   \
      detailed_log_truncated_(LOG_ALERT, __FILE__, __LINE__, args); \
    }                                                               \
  } while (false)

/** Log a critical event. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_crit(args...)                                 \
  do {                                                             \
    if (LOG_LEVEL >= LOG_CRIT) {                                   \
      detailed_log_truncated_(LOG_CRIT, __FILE__, __LINE__, args); \
    }                                                              \
  } while (false)

/** Log an error. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_error(args...)                                 \
  do {                                                              \
    if (LOG_LEVEL >= LOG_ERROR) {                                   \
      detailed_log_truncated_(LOG_ERROR, __FILE__, __LINE__, args); \
    }                                                               \
  } while (false)

/** Log a warning. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_warn(args...)                                 \
  do {                                                             \
    if (LOG_LEVEL >= LOG_WARN) {                                   \
      detailed_log_truncated_(LOG_WARN, __FILE__, __LINE__, args); \
    }                                                              \
  } while (false)

/** Log a notice. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_notice(args...)                                 \
  do {                                                               \
    if (LOG_LEVEL >= LOG_NOTICE) {                                   \
      detailed_log_truncated_(LOG_NOTICE, __FILE__, __LINE__, args); \
    }                                                                \
  } while (false)

/** Log an information message. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_info(args...)                                 \
  do {                                                             \
    if (LOG_LEVEL >= LOG_INFO) {                                   \
      detailed_log_truncated_(LOG_INFO, __FILE__, __LINE__, args); \
    }                                                              \
  } while (false)

/** Log a debugging message. (with file path and line number).
 * \param args `printf` style format and arguments.
 */
#define detailed_log_debug(args...)                                 \
  do {                                                              \
    if (LOG_LEVEL >= LOG_DEBUG) {                                   \
      detailed_log_truncated_(LOG_DEBUG, __FILE__, __LINE__, args); \
    }                                                               \
  } while (false)

/** Log a debug message indicating entry to a function.
 * Logs a debug message of the form `\<function_name\>` to indicate entry to a
 * function. `function_name` is automatically filled in with the name of the
 * current function by GCC magic.
 */
#define DEBUG_ENTRY()                \
  do {                               \
    if (DEBUG) {                     \
      log_debug("<%s>\n", __func__); \
    }                                \
  } while (false)

/** Log a debug message indicating exit to a function.
 * Logs a debug message of the form `\</function_name\>` to indicate exit from a
 * function. `function_name` is automatically filled in with the name of the
 * current function by GCC magic.
 */
#define DEBUG_EXIT()                  \
  do {                                \
    if (DEBUG) {                      \
      log_debug("</%s>\n", __func__); \
    }                                 \
  } while (false)

/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_LOGGING_H */
