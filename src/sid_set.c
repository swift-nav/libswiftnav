/*
 * Copyright (C) 2016 Swift Navigation Inc.
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
#include <string.h>

#include <libswiftnav/bits.h>
#include <libswiftnav/sid_set.h>

/** Initialize new sid set
 *
 * \param sid_set gnss_sid_set_t to be initialized
 *
 */
void sid_set_init(gnss_sid_set_t *sid_set) {
  memset(sid_set->sats, 0, sizeof(sid_set->sats));
}

/** Add new element to the set
 *
 * \param sid_set gnss_sid_set_t to add new element to
 * \param sid element to be added
 *
 */
void sid_set_add(gnss_sid_set_t *sid_set, gnss_signal_t sid) {
  u16 s = sid_to_code_index(sid);
  assert(s < 64);
  sid_set->sats[sid.code] |= ((u64)0x01 << s);
}

/** Remove an element from the set
 *
 * \param sid_set gnss_sid_set_t to remove from
 * \param sid element to be removed
 *
 */
void sid_set_remove(gnss_sid_set_t *sid_set, gnss_signal_t sid) {
  u16 s = sid_to_code_index(sid);
  assert(s < 64);
  sid_set->sats[sid.code] &= ~((u64)0x01 << s);
}

/** Get number of distinct satellites in sid set
 *
 * \param sid_set gnss_sid_set_t to count number of satellites in
 *
 * \returns Number of unique satellites present in the set.
 *
 */
u32 sid_set_get_sat_count(const gnss_sid_set_t *sid_set) {
  u64 sats[CONSTELLATION_COUNT];
  memset(sats, 0, sizeof(sats));
  for (code_t code = 0; code < CODE_COUNT; code++) {
    sats[code_to_constellation(code)] |= sid_set->sats[code];
  }

  u32 cnt = 0;
  for (constellation_t constellation = 0; constellation < CONSTELLATION_COUNT;
       constellation++) {
    cnt += count_bits_u64(sats[constellation], 1);
  }

  return cnt;
}

/** Get number of signals in sid set
 *
 * \param sid_set gnss_sid_set_t to count number of signals in
 *
 * \returns Number of signals present in the set.
 *
 */
u32 sid_set_get_sig_count(const gnss_sid_set_t *sid_set) {
  u32 cnt = 0;
  for (code_t code = 0; code < CODE_COUNT; code++) {
    cnt += count_bits_u64(sid_set->sats[code], 1);
  }

  return cnt;
}

/** Does a sid set contain given sid
 *
 * \param sid_set gnss_sid_set_t to search from
 * \param sid The signal to search for
 *
 * \returns True if sid is in sid_set
 *
 */
bool sid_set_contains(const gnss_sid_set_t *sid_set, gnss_signal_t sid) {
  u16 s = sid_to_code_index(sid);
  assert(s < 64);
  return sid_set->sats[sid.code] & ((u64)0x01 << s);
}
