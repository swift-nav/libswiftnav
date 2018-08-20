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

void correct_iono(const double *pos_ecef,
                  const ionosphere_t *iono_params,
                  u8 n_meas,
                  navigation_measurement_t *nav_meas) {
  double pos_llh[3];
  wgsecef2llh(pos_ecef, pos_llh);

  for (u8 i = 0; i < n_meas; i++) {
    double az, el, cp_correction;
    double iono_correction = 0;
    double sat_pos0[3];
    double iono_correction_delta = 0;
    double az0, el0;
    double h = 1.0; /* length of the finite difference, seconds */
    double doppler = 0.0;

    /* use doppler to increase correction accuracy if available,
     * otherwise just use the carrier frequency */
    if (0 != (nav_meas[i].flags & NAV_MEAS_FLAG_COMP_DOPPLER_VALID)) {
      doppler = nav_meas[i].computed_doppler;
    } else if (0 != (nav_meas[i].flags & NAV_MEAS_FLAG_MEAS_DOPPLER_VALID)) {
      doppler = nav_meas[i].measured_doppler;
    }

    /* this signal's frequency */
    double carrier_freq = sid_to_carr_freq(nav_meas[i].sid) + doppler;

    /* calculate azimuth and elevation of SV */
    wgsecef2azel(nav_meas[i].sat_pos, pos_ecef, &az, &el);
    /* calculate azimuth and elevation h seconds ago */
    for (u8 j = 0; j < 3; j++) {
      sat_pos0[j] = nav_meas[i].sat_pos[j] - h * nav_meas[i].sat_vel[j];
    }
    wgsecef2azel(sat_pos0, pos_ecef, &az0, &el0);

    /* calc iono correction if available */
    if (iono_params) {
      /* calculate iono correction */
      iono_correction = calc_ionosphere(
          &nav_meas[i].tot, pos_llh[0], pos_llh[1], az, el, iono_params);

      /* finite differences estimate of iono correction time derivative */
      iono_correction_delta =
          (iono_correction - calc_ionosphere(&nav_meas[i].tot,
                                             pos_llh[0],
                                             pos_llh[1],
                                             az0,
                                             el0,
                                             iono_params)) /
          h;

      if (nav_meas[i].sid.code != CODE_GPS_L1CA) {
        /* convert from L1CA Klobuchar correction */
        iono_correction *= GPS_L1_HZ * GPS_L1_HZ / carrier_freq / carrier_freq;
        iono_correction_delta *=
            GPS_L1_HZ * GPS_L1_HZ / carrier_freq / carrier_freq;
      }

      /* correct pseudorange */
      nav_meas[i].pseudorange -= iono_correction;
      /* correct carrier phase based on iono correction */
      cp_correction = iono_correction * carrier_freq / GPS_C;
      nav_meas[i].carrier_phase -= cp_correction;
      /* correct Doppler (sign opposite to pseudorange correction) */
      nav_meas[i].measured_doppler +=
          iono_correction_delta * carrier_freq / GPS_C;
      nav_meas[i].computed_doppler +=
          iono_correction_delta * carrier_freq / GPS_C;
      log_debug("%u: I %10.5f", i, iono_correction);
    }
  }
}

void correct_tropo(const double *pos_ecef,
                   u8 n_meas,
                   navigation_measurement_t *nav_meas) {
  double pos_llh[3];
  wgsecef2llh(pos_ecef, pos_llh);

  for (u8 i = 0; i < n_meas; i++) {
    double az, el, cp_correction;
    double tropo_correction = 0;
    double sat_pos0[3];
    double tropo_correction_delta = 0;
    double az0, el0;
    double h = 1.0; /* length of the finite difference, seconds */
    double doppler = 0.0;

    /* use doppler to increase correction accuracy if available,
     * otherwise just use the carrier frequency */
    if (0 != (nav_meas[i].flags & NAV_MEAS_FLAG_COMP_DOPPLER_VALID)) {
      doppler = nav_meas[i].computed_doppler;
    } else if (0 != (nav_meas[i].flags & NAV_MEAS_FLAG_MEAS_DOPPLER_VALID)) {
      doppler = nav_meas[i].measured_doppler;
    }

    /* this signal's frequency */
    double carrier_freq = sid_to_carr_freq(nav_meas[i].sid) + doppler;

    /* calculate azimuth and elevation of SV */
    wgsecef2azel(nav_meas[i].sat_pos, pos_ecef, &az, &el);
    /* calculate azimuth and elevation h seconds ago */
    for (u8 j = 0; j < 3; j++) {
      sat_pos0[j] = nav_meas[i].sat_pos[j] - h * nav_meas[i].sat_vel[j];
    }
    wgsecef2azel(sat_pos0, pos_ecef, &az0, &el0);

    /* calculate tropo correction.
     * ellipsoidal height is used due to lack of a geoid model */
    tropo_correction =
        calc_troposphere(&nav_meas[i].tot, pos_llh[0], pos_llh[2], el);
    /* finite differences estimate of tropo correction time derivative */
    tropo_correction_delta =
        (tropo_correction -
         calc_troposphere(&nav_meas[i].tot, pos_llh[0], pos_llh[2], el0)) /
        h;
    /* correct pseudorange */
    nav_meas[i].pseudorange -= tropo_correction;
    /* correct carrier phase based on tropo correction */
    cp_correction = tropo_correction * carrier_freq / GPS_C;
    /* sign here is opposite to normal due to
     * Piksi's unusual sign convention on carrier phase */
    nav_meas[i].carrier_phase += cp_correction;
    /* correct Doppler (sign opposite to pseudorange correction) */
    nav_meas[i].measured_doppler +=
        tropo_correction_delta * carrier_freq / GPS_C;
    nav_meas[i].computed_doppler +=
        tropo_correction_delta * carrier_freq / GPS_C;
    log_debug("%u: T %10.5f", i, tropo_correction);
  }
}
