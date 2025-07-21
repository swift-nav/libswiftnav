/*
 * Copyright (c) 2018 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <swiftnav/constants.h>
#include <swiftnav/correct_iono_tropo.h>
#include <swiftnav/ionosphere.h>
#include <swiftnav/troposphere.h>

/* calculate azimuth and elevation at 't' seconds ago */
static inline void calculate_prev_sat_pos(
    const navigation_measurement_t* nav_meas,
    const double t,
    const double* pos_ecef,
    double* az,
    double* el) {
  double sat_pos0[3];

  for (u8 j = 0; j < 3; j++) {
    sat_pos0[j] = nav_meas->sat_pos[j] - (t * nav_meas->sat_vel[j]);
  }
  wgsecef2azel(sat_pos0, pos_ecef, az, el);
}

void correct_iono(const double* pos_ecef,
                  const ionosphere_t* iono_params,
                  u8 n_meas,
                  navigation_measurement_t* nav_meas) {
  double az;
  double el;
  double az0;
  double el0;
  static const double h = 1.0; /* length of the finite difference, seconds */

  if (!iono_params) {
    return;
  }

  double pos_llh[3];
  wgsecef2llh(pos_ecef, pos_llh);

  for (u8 i = 0; i < n_meas; i++) {
    /* this signal's frequency */
    double carrier_freq = sid_to_carr_freq(nav_meas[i].sid);

    /* calculate current azimuth and elevation of SV */
    wgsecef2azel(nav_meas[i].sat_pos, pos_ecef, &az, &el);
    /* calculate past azimuth and elevation of SV */
    calculate_prev_sat_pos(&nav_meas[i], h, pos_ecef, &az0, &el0);

    /* calculate iono correction */
    double iono_correction = calc_ionosphere(
        &nav_meas[i].tot, pos_llh[0], pos_llh[1], az, el, iono_params);

    /* finite differences estimate of iono correction time derivative */
    double iono_correction_delta =
        (iono_correction -
         calc_ionosphere(
             &nav_meas[i].tot, pos_llh[0], pos_llh[1], az0, el0, iono_params)) /
        h;

    /* convert from L1CA Klobuchar correction */
    iono_correction *= (GPS_L1_HZ / carrier_freq) * (GPS_L1_HZ / carrier_freq);
    iono_correction_delta *=
        (GPS_L1_HZ / carrier_freq) * (GPS_L1_HZ / carrier_freq);

    /* correct pseudorange */
    nav_meas[i].raw_pseudorange -= iono_correction;
    /* correct carrier phase based on iono correction */
    double cp_correction = iono_correction * (carrier_freq / GPS_C);
    nav_meas[i].raw_carrier_phase -= cp_correction;
    /* correct Doppler (sign opposite to pseudorange correction) */
    nav_meas[i].raw_measured_doppler +=
        iono_correction_delta * (carrier_freq / GPS_C);
    nav_meas[i].raw_computed_doppler +=
        iono_correction_delta * (carrier_freq / GPS_C);
    log_debug("%u: I %10.5f", i, iono_correction);
  }
}

void correct_tropo(const double* pos_ecef,
                   u8 n_meas,
                   navigation_measurement_t* nav_meas) {
  double az;
  double el;
  double az0;
  double el0;
  static const double h = 1.0; /* length of the finite difference, seconds */

  double pos_llh[3];
  wgsecef2llh(pos_ecef, pos_llh);

  for (u8 i = 0; i < n_meas; i++) {
    /* this signal's frequency */
    double carrier_freq = sid_to_carr_freq(nav_meas[i].sid);

    /* calculate current azimuth and elevation of SV */
    wgsecef2azel(nav_meas[i].sat_pos, pos_ecef, &az, &el);
    /* calculate past azimuth and elevation of SV */
    calculate_prev_sat_pos(&nav_meas[i], h, pos_ecef, &az0, &el0);

    /* compute day of year from gps time */
    double doy = (double)gps2doy(&nav_meas[i].tot);

    /* calculate tropo correction.
     * ellipsoidal height is used due to lack of a geoid model */
    double tropo_correction = calc_troposphere(doy, pos_llh[0], pos_llh[2], el);
    /* finite differences estimate of tropo correction time derivative */
    double tropo_correction_delta =
        (tropo_correction -
         calc_troposphere(doy, pos_llh[0], pos_llh[2], el0)) /
        h;
    /* correct pseudorange */
    nav_meas[i].raw_pseudorange -= tropo_correction;
    /* correct carrier phase based on tropo correction */
    double cp_correction = tropo_correction * (carrier_freq / GPS_C);
    /* sign here is opposite to normal due to
     * Piksi's unusual sign convention on carrier phase */
    nav_meas[i].raw_carrier_phase += cp_correction;
    /* correct Doppler (sign opposite to pseudorange correction) */
    nav_meas[i].raw_measured_doppler +=
        tropo_correction_delta * (carrier_freq / GPS_C);
    nav_meas[i].raw_computed_doppler +=
        tropo_correction_delta * (carrier_freq / GPS_C);
    log_debug("%u: T %10.5f", i, tropo_correction);
  }
}
