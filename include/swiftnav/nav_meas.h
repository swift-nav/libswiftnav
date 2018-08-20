/*
 * Copyright (C) 2017 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_NAV_MEAS_H
#define LIBSWIFTNAV_NAV_MEAS_H

#include <swiftnav/ch_meas.h>
#include <swiftnav/common.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Measurement flag: pseudorange and raw_pseudorange fields contain a
 *  valid value.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_CODE_VALID ((nav_meas_flags_t)1 << 0)
/** Measurement flag: carrier_phase and raw_carrier_phase fields contain a
 *  valid value.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_PHASE_VALID ((nav_meas_flags_t)1 << 1)
/** Measurement flag: measured_doppler and raw_measured_doppler fields
 *  contain a valid value.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_MEAS_DOPPLER_VALID ((nav_meas_flags_t)1 << 2)
/** Measurement flag: computed_doppler and raw_computed_doppler fields
 *  contain a valid value.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_COMP_DOPPLER_VALID ((nav_meas_flags_t)1 << 3)
/** Measurement flag: if bit not set, the half cycle carrier phase ambiguity has
 *  yet to be resolved. Carrier phase measurements might be 0.5 cycles out of
 *  phase.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_HALF_CYCLE_KNOWN ((nav_meas_flags_t)1 << 4)
/** Measurement flag: cn0 field contains a valid value.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_CN0_VALID ((nav_meas_flags_t)1 << 5)
/** Measurement flag: measurement was excluded by SPP RAIM, use with care.
 *  \sa nav_meas_flags_t */
#define NAV_MEAS_FLAG_RAIM_EXCLUSION ((nav_meas_flags_t)1 << 6)

/** Navigation measurement flag mask.
 *
 * Mask value is any combination of the following flags:
 * - #NAV_MEAS_FLAG_CODE_VALID
 * - #NAV_MEAS_FLAG_PHASE_VALID
 * - #NAV_MEAS_FLAG_MEAS_DOPPLER_VALID
 * - #NAV_MEAS_FLAG_COMP_DOPPLER_VALID
 * - #NAV_MEAS_FLAG_HALF_CYCLE_KNOWN
 * - #NAV_MEAS_FLAG_CN0_VALID
 *
 * \sa navigation_measurement_t
 */
typedef u16 nav_meas_flags_t;

/**
 * Structure for processing navigation measurements
 */
typedef struct {
  double raw_pseudorange;      /**< Raw pseudorange: time of flight
                                *   multiplied by speed of light [m] */
  double pseudorange;          /**< Corrected pseudorange [m] */
  double raw_carrier_phase;    /**< Raw carrier phase [cycle] */
  double carrier_phase;        /**< Corrected carrier phase [cycle] */
  double raw_measured_doppler; /**< Raw doppler from tracker [Hz] */
  double measured_doppler;     /**< Corrected doppler from tracker [Hz]*/
  double raw_computed_doppler; /**< Raw doppler from time difference of
                                *   carrier phase [Hz] */
  double computed_doppler;     /**< Corrected doppler from time
                                *   difference of carrier phase [Hz] */
  double computed_doppler_dt;  /**< Time difference for computed doppler [s] */
  double sat_pos[3];           /**< SV ECEF position [m] */
  double sat_vel[3];           /**< SV ECEF velocity [m/s] */
  double sat_acc[3];           /**< SV ECEF accel [m/s/s] */
  u8 IODE;                     /**< Issue of Data Ephemeris [unitless] */
  double sat_clock_err;        /**< SV clock error [s] */
  double sat_clock_err_rate;   /**< SV clock error rate [s/s] */
  u16 IODC;                    /**< Issue of Data Clock [unitless] */
  double cn0;                  /**< Carrier to noise ratio [dB-Hz] */
  double lock_time;            /**< PLL lock time [s] */
  double elevation;            /**< Approximate satellite elevation [deg] */
  gps_time_t tot;              /**< Time of transmit */
  gnss_signal_t sid;           /**< SV signal identifier */
  nav_meas_flags_t flags;      /**< Measurement flags */
} navigation_measurement_t;

/**
 * Structure for processing navigation measurements estimated standard deviation
 */
typedef struct {
  gnss_signal_t sid; /**< SV signal identifier */
  double iono_std;   /**< Observations ionospheric delay std [m] */
  double tropo_std;  /**< Observations tropospheric delay std [m] */
  double range_std;  /**< Observations orbit/clock delay std [m] */
  u8 flags;          /**< Observations fixing flags [m] */
} measurement_std_t;

int nav_meas_cmp(const void *a, const void *b);
bool nav_meas_flags_valid(nav_meas_flags_t flags);
bool pseudorange_valid(navigation_measurement_t meas);

u8 encode_lock_time(double nm_lock_time);
double decode_lock_time(u8 sbp_lock_time);

static inline bool not_l2p_sid(navigation_measurement_t a) {
  return a.sid.code != CODE_GPS_L2P;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_NAV_MEAS_H */
