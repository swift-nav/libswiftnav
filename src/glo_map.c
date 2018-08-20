/*
 * Copyright (c) 2017 Swift Navigation Inc.
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

#include <swiftnav/glo_map.h>
#include <swiftnav/signal.h>

#define NUM_GLO_MAP_INDICES (NUM_SATS_GLO + 1)

/* GLO to FCN look up table, index 0 unused, index 1 -- SV 1, index 28 -- SV 28
 */
static u8 glo_sv_id_fcn_map[NUM_GLO_MAP_INDICES] = {GLO_FCN_UNKNOWN};
static void (*lock)(void) = NULL;
static void (*unlock)(void) = NULL;

/** Locks glo_sv_id_fcn_map for multithread access */
static void glo_map_lock(void) {
  if (lock) {
    lock();
  }
}

/** Unlocks glo_sv_id_fcn_map for multithread access */
static void glo_map_unlock(void) {
  if (unlock) {
    unlock();
  }
}

/** Init GLO map
 * @param lock_cb Callback function for mutual exclusion locking
 * @param unlock_cb Callback function for mutual exclusion unlocking
 */
void glo_map_init(void (*lock_cb)(void), void (*unlock_cb)(void)) {
  lock = lock_cb;
  unlock = unlock_cb;
}

/** GLO map validity predicate.
 * @param sid Signal identifier
 * @retval true The mapping is valid
 * @retval false The mapping is invalid
 */
bool glo_map_valid(const gnss_signal_t sid) {
  assert(IS_GLO(sid));
  assert(glo_slot_id_is_valid(sid.sat));

  u16 fcn = (u16)glo_sv_id_fcn_map[sid.sat];
  bool valid = (fcn != GLO_FCN_UNKNOWN);

  return valid;
}

/** The function maps GLO orbital slot and frequency slot
 *
 * @param[in] mesid ME signal identifier
 * @param[in] glo_slot_id GLO orbital slot
 */
void glo_map_set_slot_id(me_gnss_signal_t mesid, u16 glo_slot_id) {
  assert(IS_GLO(mesid));
  assert(glo_slot_id_is_valid(glo_slot_id));
  assert(glo_fcn_is_valid(mesid.sat));

  glo_map_lock();
  glo_sv_id_fcn_map[glo_slot_id] = mesid.sat;
  glo_map_unlock();
}

/** The function returns GLO frequency slot corresponds to the GLO SV ID
 *
 * @param[in] sid Signal identifier
 * @return GLO frequency slot corresponds to the GLO SV ID (1..14)
 */
u16 glo_map_get_fcn(gnss_signal_t sid) {
  assert(IS_GLO(sid));
  assert(glo_slot_id_is_valid(sid.sat));

  u16 fcn = (u16)glo_sv_id_fcn_map[sid.sat];

  assert(fcn != GLO_FCN_UNKNOWN);

  return fcn;
}

/** The function clears mapping between GLO SV ID and GLO FCN
 *
 * @param glo_slot_id GLO orbital slot
 */
void glo_map_clear_slot_id(u16 glo_slot_id) {
  assert(glo_slot_id_is_valid(glo_slot_id));

  glo_map_lock();
  glo_sv_id_fcn_map[glo_slot_id] = GLO_FCN_UNKNOWN;
  glo_map_unlock();
}

/** This function clears the entire mapping between GLO SV ID and GLO FCN. */
void glo_map_clear_all(void) {
  for (u16 i = 1; i < NUM_GLO_MAP_INDICES; ++i) {
    glo_map_clear_slot_id(i);
  }
}

/** This function fills the glo_map with dummy data so unit tests which use
 * GLONASS observations (but don't rely on actual wavelength values) can run.
 */
void glo_map_fill_dummy_data(void) {
  for (u16 i = 1; i < NUM_GLO_MAP_INDICES; ++i) {
    glo_sv_id_fcn_map[i] = -1;
  }
}

/**
 * The function checks if the FCN mapped to any slot ID.
 *
 * @param[in]  fcn      Frequency slot to be checked
 * @param[out] slot_id1 Pointer to the first slot ID container
 * @param[out] slot_id2 Pointer to the second slot ID container.
 *                      Function write 0 to the container if there is only one
 *                      slot ID mapped to the frequency
 * @return number of slot IDs mapped to the frequency slot in question
 *         valid values are [0..2]
 */
u8 glo_map_get_slot_id(const u16 fcn, u16 *slot_id1, u16 *slot_id2) {
  assert(slot_id1 != NULL && slot_id2 != NULL);
  u8 si_num = 0;
  *slot_id1 = 0;
  *slot_id2 = 0;
  for (u8 i = GLO_FIRST_PRN; i < NUM_GLO_MAP_INDICES; i++) {
    if (fcn == glo_sv_id_fcn_map[i]) {
      /* the fcn mapped, so write to output */
      si_num++;
      if (si_num == 1) {
        /* 1st slot found */
        *slot_id1 = i;
      }
      if (si_num == 2) {
        /* 2 slots id found no need to continue */
        *slot_id2 = i;
        break;
      }
    }
  }
  return si_num;
}
