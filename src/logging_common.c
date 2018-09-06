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

const char *level_string[] = {
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
};

/** Shortens the full path to the filename only.
 *
 * \param  path string of full path to file where this function was called
 * @return base file name of the path
 */
const char *truncate_path_(char *path) {
  assert(NULL != path);
  int i;

  if (path[0] == '\0') return "";
  for (i = strlen(path) - 1; i >= 0 && path[i] == '/'; i--)
    ;
  if (i == -1) return "/";
  // set the trailing character to null in case it was '/'
  // e.g. /dev/null/
  path[i + 1] = '\0';
  // Go backwards until the prior '/'
  while (i >= 0 && path[i] != '/') i--;
  // Return a pointer to the remainder
  return &path[i + 1];
}
