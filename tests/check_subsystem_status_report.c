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

#include <check.h>
#include <stdbool.h>
#include <swiftnav/subsystem_status_report.h>

#include "check_suites.h"

typedef struct {
  size_t invocations;
  uint16_t component;
  uint8_t generic;
  uint8_t specific;
  void *context;
  size_t counter;
} callback_tracker_t;

static size_t counter_tracker;
static callback_tracker_t callback_tracker[3];

static void callback1(uint16_t component,
                      uint8_t generic,
                      uint8_t specific,
                      void *context) {
  const size_t index = 0;
  callback_tracker[index].invocations++;
  callback_tracker[index].component = component;
  callback_tracker[index].generic = generic;
  callback_tracker[index].specific = specific;
  callback_tracker[index].context = context;
  callback_tracker[index].counter = counter_tracker++;
}

static void callback2(uint16_t component,
                      uint8_t generic,
                      uint8_t specific,
                      void *context) {
  const size_t index = 1;
  callback_tracker[index].invocations++;
  callback_tracker[index].component = component;
  callback_tracker[index].generic = generic;
  callback_tracker[index].specific = specific;
  callback_tracker[index].context = context;
  callback_tracker[index].counter = counter_tracker++;
}

static void callback3(uint16_t component,
                      uint8_t generic,
                      uint8_t specific,
                      void *context) {
  const size_t index = 2;
  callback_tracker[index].invocations++;
  callback_tracker[index].component = component;
  callback_tracker[index].generic = generic;
  callback_tracker[index].specific = specific;
  callback_tracker[index].context = context;
  callback_tracker[index].counter = counter_tracker++;
}

static void fixture_teardown() {
  counter_tracker = 0;
  memset(&callback_tracker, 0, sizeof(callback_tracker));
  swiftnav_subsystem_status_report_callback_reset();
}

START_TEST(test_no_registered_callbacks) {
  swiftnav_send_subsystem_status_report(0, 1, 2);

  ck_assert_uint_eq(callback_tracker[0].invocations, 0);
  ck_assert_uint_eq(callback_tracker[1].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 0);
}
END_TEST

START_TEST(test_one_registered_callbacks) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_send_subsystem_status_report(0, 1, 2);

  ck_assert_uint_eq(callback_tracker[0].invocations, 1);
  ck_assert_uint_eq(callback_tracker[0].component, 0);
  ck_assert_uint_eq(callback_tracker[0].generic, 1);
  ck_assert_uint_eq(callback_tracker[0].specific, 2);
  ck_assert_ptr_eq(callback_tracker[0].context, (void *)0xff);
  ck_assert_uint_eq(callback_tracker[0].counter, 0);

  ck_assert_uint_eq(callback_tracker[1].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 0);
}
END_TEST

START_TEST(test_two_registered_callbacks) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_send_subsystem_status_report(0, 1, 2);

  ck_assert_uint_eq(callback_tracker[0].invocations, 1);
  ck_assert_uint_eq(callback_tracker[0].component, 0);
  ck_assert_uint_eq(callback_tracker[0].generic, 1);
  ck_assert_uint_eq(callback_tracker[0].specific, 2);
  ck_assert_ptr_eq(callback_tracker[0].context, (void *)0xff);
  ck_assert_uint_eq(callback_tracker[0].counter, 0);

  ck_assert_uint_eq(callback_tracker[1].invocations, 1);
  ck_assert_uint_eq(callback_tracker[1].component, 0);
  ck_assert_uint_eq(callback_tracker[1].generic, 1);
  ck_assert_uint_eq(callback_tracker[1].specific, 2);
  ck_assert_ptr_eq(callback_tracker[1].context, (void *)0xee);
  ck_assert_uint_eq(callback_tracker[1].counter, 1);

  ck_assert_uint_eq(callback_tracker[2].invocations, 0);
}
END_TEST

START_TEST(test_no_registered_callbacks_deregister) {
  swiftnav_subsystem_status_report_callback_deregister(NULL);

  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_deregister(&node1);
}
END_TEST

START_TEST(test_one_registered_callbacks_deregister) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_deregister(NULL);
  swiftnav_subsystem_status_report_callback_deregister(&node1);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  ck_assert_uint_eq(callback_tracker[0].invocations, 0);
  ck_assert_uint_eq(callback_tracker[1].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 0);
}
END_TEST

START_TEST(test_two_registered_callbacks_deregister_first) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_deregister(NULL);
  swiftnav_subsystem_status_report_callback_deregister(&node1);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  ck_assert_uint_eq(callback_tracker[0].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 0);

  ck_assert_uint_eq(callback_tracker[1].invocations, 1);
  ck_assert_uint_eq(callback_tracker[1].component, 0);
  ck_assert_uint_eq(callback_tracker[1].generic, 1);
  ck_assert_uint_eq(callback_tracker[1].specific, 2);
  ck_assert_ptr_eq(callback_tracker[1].context, (void *)0xee);
  ck_assert_uint_eq(callback_tracker[1].counter, 0);
}
END_TEST

START_TEST(test_two_registered_callbacks_deregister_second) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_deregister(NULL);
  swiftnav_subsystem_status_report_callback_deregister(&node2);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  ck_assert_uint_eq(callback_tracker[0].invocations, 1);
  ck_assert_uint_eq(callback_tracker[1].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 0);

  ck_assert_uint_eq(callback_tracker[0].component, 0);
  ck_assert_uint_eq(callback_tracker[0].generic, 1);
  ck_assert_uint_eq(callback_tracker[0].specific, 2);
  ck_assert_ptr_eq(callback_tracker[0].context, (void *)0xff);
  ck_assert_uint_eq(callback_tracker[0].counter, 0);
}
END_TEST

START_TEST(test_three_registered_callbacks_deregister_second) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_node_t node3;
  swiftnav_subsystem_status_report_callback_register(
      &node3, callback3, (void *)0xdd);

  swiftnav_subsystem_status_report_callback_deregister(NULL);
  swiftnav_subsystem_status_report_callback_deregister(&node2);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  ck_assert_uint_eq(callback_tracker[0].invocations, 1);
  ck_assert_uint_eq(callback_tracker[1].invocations, 0);
  ck_assert_uint_eq(callback_tracker[2].invocations, 1);

  ck_assert_uint_eq(callback_tracker[0].component, 0);
  ck_assert_uint_eq(callback_tracker[0].generic, 1);
  ck_assert_uint_eq(callback_tracker[0].specific, 2);
  ck_assert_ptr_eq(callback_tracker[0].context, (void *)0xff);
  ck_assert_uint_eq(callback_tracker[0].counter, 0);

  ck_assert_uint_eq(callback_tracker[2].component, 0);
  ck_assert_uint_eq(callback_tracker[2].generic, 1);
  ck_assert_uint_eq(callback_tracker[2].specific, 2);
  ck_assert_ptr_eq(callback_tracker[2].context, (void *)0xdd);
  ck_assert_uint_eq(callback_tracker[2].counter, 1);
}
END_TEST

Suite *status_report_suite(void) {
  Suite *suite = suite_create("Status Report");

  TCase *test_cases = tcase_create("Core");
  tcase_add_checked_fixture(test_cases, NULL, fixture_teardown);
  tcase_add_test(test_cases, test_no_registered_callbacks);
  tcase_add_test(test_cases, test_one_registered_callbacks);
  tcase_add_test(test_cases, test_two_registered_callbacks);
  tcase_add_test(test_cases, test_no_registered_callbacks_deregister);
  tcase_add_test(test_cases, test_one_registered_callbacks_deregister);
  tcase_add_test(test_cases, test_two_registered_callbacks_deregister_first);
  tcase_add_test(test_cases, test_two_registered_callbacks_deregister_second);
  tcase_add_test(test_cases, test_three_registered_callbacks_deregister_second);
  suite_add_tcase(suite, test_cases);

  return suite;
}
