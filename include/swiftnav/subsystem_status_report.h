/**
 * Copyright (C) 2022 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_SUBSYSTEM_STATUS_REPORT_H
#define LIBSWIFTNAV_SUBSYSTEM_STATUS_REPORT_H

#include <inttypes.h>
#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pfn_subsystem_status_report)(uint16_t component,
                                            uint8_t generic,
                                            uint8_t specific,
                                            void* context);

/**
 * Struct used by the "swiftnav_subsystem_status_report_callback_register" to
 * register a callback function along with context object to be invoked whenever
 * someone invokes "swiftnav_send_subsystem_status_report" from anywhere. Struct
 * should not be modified by the user and should be treated as an opaque type.
 */
typedef struct swiftnav_subsystem_status_report_callback_node {
  pfn_subsystem_status_report callback;
  void* context;
  struct swiftnav_subsystem_status_report_callback_node* next;
} swiftnav_subsystem_status_report_callback_node_t;

/**
 * Specifies the callback function to invoke when a user calls the
 * "swiftnav_send_subsystem_status_report" function.
 *
 * @param callback_node pointer to opaque struct which user should pass to the
 * function in order for it to register the subsequent callback function and
 * context object
 * @param callback callback function to register, if the value is NULL the
 * function will deregister prior callback
 * @param context context object which will be passed to the callback function
 * when invoked
 */
void swiftnav_subsystem_status_report_callback_register(
    swiftnav_subsystem_status_report_callback_node_t* callback_node,
    pfn_subsystem_status_report callback,
    void* context);

/**
 * De-registers the previously registered callback node.
 *
 * @param callback_node pointer to opaque struct which user's have previously
 * passed into the "swiftnav_subsystem_status_report_callback_register"
 * function.
 *
 * @see swiftnav_subsystem_status_report_callback_register
 */
void swiftnav_subsystem_status_report_callback_deregister(
    swiftnav_subsystem_status_report_callback_node_t* callback_node);

/**
 * De-registers all prior registered callbacks.
 *
 * @see swiftnav_subsystem_status_report_callback_register
 */
void swiftnav_subsystem_status_report_callback_reset(void);

/**
 * Invoking this function will indirectly call the registered callback function
 * specified via "swiftnav_subsystem_status_report_callback_register".
 *
 * @param component identity of reporting subsystem
 * @param generic generic form status report
 * @param specific subsystem specific status code
 */
void swiftnav_send_subsystem_status_report(uint16_t component,
                                           uint8_t generic,
                                           uint8_t specific);

#ifdef __cplusplus
}
#endif

#endif  // LIBSWIFTNAV_SUBSYSTEM_STATUS_REPORT_H
