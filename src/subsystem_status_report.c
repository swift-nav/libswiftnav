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

#include <stddef.h>
#include <swiftnav/subsystem_status_report.h>

static swiftnav_subsystem_status_report_callback_node_t* root_callback_node =
    NULL;

void swiftnav_subsystem_status_report_callback_register(
    swiftnav_subsystem_status_report_callback_node_t* callback_node,
    pfn_subsystem_status_report callback,
    void* context) {
  callback_node->callback = callback;
  callback_node->context = context;
  callback_node->next = NULL;

  swiftnav_subsystem_status_report_callback_node_t* previous = NULL;
  swiftnav_subsystem_status_report_callback_node_t* current =
      root_callback_node;

  for (; current != NULL; previous = current, current = current->next) {
  }

  if (previous != NULL) {
    previous->next = callback_node;
  } else {
    root_callback_node = callback_node;
  }
}

void swiftnav_subsystem_status_report_callback_deregister(
    swiftnav_subsystem_status_report_callback_node_t* callback_node) {
  swiftnav_subsystem_status_report_callback_node_t* previous = NULL;
  swiftnav_subsystem_status_report_callback_node_t* current =
      root_callback_node;

  for (; current != NULL; previous = current, current = current->next) {
    if (current != callback_node) {
      continue;
    }

    if (previous != NULL) {
      previous->next = current->next;
    } else {
      root_callback_node = current->next;
    }
  }
}

void swiftnav_subsystem_status_report_callback_reset(void) {
  root_callback_node = NULL;
}

void swiftnav_send_subsystem_status_report(uint16_t component,
                                           uint8_t generic,
                                           uint8_t specific) {
  swiftnav_subsystem_status_report_callback_node_t* node = root_callback_node;
  for (; node != NULL; node = node->next) {
    node->callback(component, generic, specific, node->context);
  }
}
