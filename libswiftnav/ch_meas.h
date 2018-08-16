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

#ifndef LIBSWIFTNAV_CH_MEAS_H
#define LIBSWIFTNAV_CH_MEAS_H

#include <libswiftnav/common.h>
#include <libswiftnav/signal.h>

/** Measurement flag: code measurement is valid.
 * \sa chan_meas_flags_t */
#define CHAN_MEAS_FLAG_CODE_VALID ((chan_meas_flags_t)1 << 0)
/** Measurement flag: phase measurement is valid.
 * \sa chan_meas_flags_t */
#define CHAN_MEAS_FLAG_PHASE_VALID ((chan_meas_flags_t)1 << 1)
/** Measurement flag: doppler measurement is valid.
 * \sa chan_meas_flags_t */
#define CHAN_MEAS_FLAG_MEAS_DOPPLER_VALID ((chan_meas_flags_t)1 << 2)
/** Measurement flag: phase measurement has a known polarity.
 * \sa chan_meas_flags_t */
#define CHAN_MEAS_FLAG_HALF_CYCLE_KNOWN ((chan_meas_flags_t)1 << 3)

/** Channel measurement flag mask.
 *
 * Mask value is any combination of the following flags:
 * - #CHAN_MEAS_FLAG_CODE_VALID
 * - #CHAN_MEAS_FLAG_PHASE_VALID
 * - #CHAN_MEAS_FLAG_MEAS_DOPPLER_VALID
 * - #CHAN_MEAS_FLAG_HALF_CYCLE_KNOWN
 *
 * \sa channel_measurement_t
 */
typedef u16 chan_meas_flags_t;

/** This struct holds the state of a tracking channel at a given receiver time
 * epoch.
 *
 * The struct contains the information necessary to calculate the pseudorange,
 * carrier phase and Doppler information needed for a PVT solution but is
 * formatted closer to the natural outputs from the tracking channels.
 *
 * \see calc_navigation_measurement()
 */
typedef struct {
  gnss_signal_t sid;       /**< Satellite signal. */
  double code_phase_chips; /**< The code-phase in chips at `receiver_time`. */
  double code_phase_rate;  /**< Code phase rate in chips/s. */
  double carrier_phase;    /**< Carrier phase in cycles. */
  double carrier_freq;     /**< Carrier frequency in Hz. */
  u32 time_of_week_ms;     /**< Number of milliseconds since the start of the
                                GPS week corresponding to the last code
                                rollover. */
  s32 tow_residual_ns;     /**< Residual to time_of_week_ms in [ns] */
  double rec_time_delta;   /**< Difference between receiver clock time at which
                                this measurement is valid and reference time
                                (seconds). */
  double cn0;              /**< Carrier to noise ratio in dB-Hz. */
  double lock_time;        /**< PLL Lock time in seconds. Counts the time since
                                the tracking channel re-locked or a cycle
                                slip was detected and the carrier phase
                                integer ambiguity was reset. If this value
                                drops it is an indication you should reset
                                integer ambiguity resolution for this
                                channel. */
  double time_in_track;    /**< Time the pseudorange has been tracked (s) */
  double elevation;        /**< Approximate satellite elevation (deg) */
  chan_meas_flags_t flags; /**< Flags */
} channel_measurement_t;

#endif /* LIBSWIFTNAV_CH_MEAS_H */
