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

#include <gtest/gtest.h>
#include <stdbool.h>
#include <swiftnav/subsystem_status_report.h>

namespace {

typedef struct {
  size_t invocations;
  uint16_t component;
  uint8_t generic;
  uint8_t specific;
  void *context;
  size_t counter;
} callback_tracker_t;

size_t counter_tracker;
callback_tracker_t callback_tracker[3];

void callback1(uint16_t component,
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

void callback2(uint16_t component,
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

void callback3(uint16_t component,
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

void fixture_teardown() {
  counter_tracker = 0;
  memset(&callback_tracker, 0, sizeof(callback_tracker));
  swiftnav_subsystem_status_report_callback_reset();
}

class TestSubsystemStatusReport : public ::testing::Test {
 protected:
  void TearDown() override { fixture_teardown(); }
};

TEST_F(TestSubsystemStatusReport, TestNoRegisteredCallbacks) {
  swiftnav_send_subsystem_status_report(0, 1, 2);

  EXPECT_EQ(callback_tracker[0].invocations, 0);
  EXPECT_EQ(callback_tracker[1].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 0);
}

TEST_F(TestSubsystemStatusReport, TestOneRegisteredCallbacks) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_send_subsystem_status_report(0, 1, 2);

  EXPECT_EQ(callback_tracker[0].invocations, 1);
  EXPECT_EQ(callback_tracker[0].component, 0);
  EXPECT_EQ(callback_tracker[0].generic, 1);
  EXPECT_EQ(callback_tracker[0].specific, 2);
  EXPECT_EQ(callback_tracker[0].context, (void *)0xff);
  EXPECT_EQ(callback_tracker[0].counter, 0);

  EXPECT_EQ(callback_tracker[1].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 0);
}

TEST_F(TestSubsystemStatusReport, TwoRegisteredCallbacks) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_send_subsystem_status_report(0, 1, 2);

  EXPECT_EQ(callback_tracker[0].invocations, 1);
  EXPECT_EQ(callback_tracker[0].component, 0);
  EXPECT_EQ(callback_tracker[0].generic, 1);
  EXPECT_EQ(callback_tracker[0].specific, 2);
  EXPECT_EQ(callback_tracker[0].context, (void *)0xff);
  EXPECT_EQ(callback_tracker[0].counter, 0);

  EXPECT_EQ(callback_tracker[1].invocations, 1);
  EXPECT_EQ(callback_tracker[1].component, 0);
  EXPECT_EQ(callback_tracker[1].generic, 1);
  EXPECT_EQ(callback_tracker[1].specific, 2);
  EXPECT_EQ(callback_tracker[1].context, (void *)0xee);
  EXPECT_EQ(callback_tracker[1].counter, 1);

  EXPECT_EQ(callback_tracker[2].invocations, 0);
}

TEST_F(TestSubsystemStatusReport, NoRegisteredCallbacksDeregister) {
  swiftnav_subsystem_status_report_callback_deregister(nullptr);

  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_deregister(&node1);
}

TEST_F(TestSubsystemStatusReport, OneRegisteredCallbacksDeregister) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_deregister(nullptr);
  swiftnav_subsystem_status_report_callback_deregister(&node1);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  EXPECT_EQ(callback_tracker[0].invocations, 0);
  EXPECT_EQ(callback_tracker[1].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 0);
}

TEST_F(TestSubsystemStatusReport, TwoRegisteredCallbacksDeregisterFirst) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_deregister(nullptr);
  swiftnav_subsystem_status_report_callback_deregister(&node1);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  EXPECT_EQ(callback_tracker[0].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 0);

  EXPECT_EQ(callback_tracker[1].invocations, 1);
  EXPECT_EQ(callback_tracker[1].component, 0);
  EXPECT_EQ(callback_tracker[1].generic, 1);
  EXPECT_EQ(callback_tracker[1].specific, 2);
  EXPECT_EQ(callback_tracker[1].context, (void *)0xee);
  EXPECT_EQ(callback_tracker[1].counter, 0);
}

TEST_F(TestSubsystemStatusReport, TwoRegisteredCallbacksDeregisterSecond) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_deregister(nullptr);
  swiftnav_subsystem_status_report_callback_deregister(&node2);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  EXPECT_EQ(callback_tracker[0].invocations, 1);
  EXPECT_EQ(callback_tracker[1].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 0);

  EXPECT_EQ(callback_tracker[0].component, 0);
  EXPECT_EQ(callback_tracker[0].generic, 1);
  EXPECT_EQ(callback_tracker[0].specific, 2);
  EXPECT_EQ(callback_tracker[0].context, (void *)0xff);
  EXPECT_EQ(callback_tracker[0].counter, 0);
}

TEST_F(TestSubsystemStatusReport, ThreeRegisteredCallbacksDeregisterSecond) {
  swiftnav_subsystem_status_report_callback_node_t node1;
  swiftnav_subsystem_status_report_callback_register(
      &node1, callback1, (void *)0xff);

  swiftnav_subsystem_status_report_callback_node_t node2;
  swiftnav_subsystem_status_report_callback_register(
      &node2, callback2, (void *)0xee);

  swiftnav_subsystem_status_report_callback_node_t node3;
  swiftnav_subsystem_status_report_callback_register(
      &node3, callback3, (void *)0xdd);

  swiftnav_subsystem_status_report_callback_deregister(nullptr);
  swiftnav_subsystem_status_report_callback_deregister(&node2);

  swiftnav_send_subsystem_status_report(0, 1, 2);
  EXPECT_EQ(callback_tracker[0].invocations, 1);
  EXPECT_EQ(callback_tracker[1].invocations, 0);
  EXPECT_EQ(callback_tracker[2].invocations, 1);

  EXPECT_EQ(callback_tracker[0].component, 0);
  EXPECT_EQ(callback_tracker[0].generic, 1);
  EXPECT_EQ(callback_tracker[0].specific, 2);
  EXPECT_EQ(callback_tracker[0].context, (void *)0xff);
  EXPECT_EQ(callback_tracker[0].counter, 0);

  EXPECT_EQ(callback_tracker[2].component, 0);
  EXPECT_EQ(callback_tracker[2].generic, 1);
  EXPECT_EQ(callback_tracker[2].specific, 2);
  EXPECT_EQ(callback_tracker[2].context, (void *)0xdd);
  EXPECT_EQ(callback_tracker[2].counter, 1);
}

}  // namespace
