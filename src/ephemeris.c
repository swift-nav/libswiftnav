/*
 * Copyright (C) 2010, 2016, 2020 Swift Navigation Inc.
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
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <swiftnav/bits.h>
#include <swiftnav/constants.h>
#include <swiftnav/coord_system.h>
#include <swiftnav/decode_glo.h>
#include <swiftnav/edc.h>
#include <swiftnav/ephemeris.h>
#include <swiftnav/float_equality.h>
#include <swiftnav/linear_algebra.h>
#include <swiftnav/logging.h>
#include <swiftnav/shm.h>

/** \defgroup ephemeris Ephemeris
 * Functions and calculations related to the GPS ephemeris.
 * \{ */

/* maximum step length in seconds for Runge-Kutta algorithm */
#define GLO_MAX_STEP_LENGTH 30

/* maximum steps for Runge-Kutta algorithm,
 * based on modeling https://github.com/swift-nav/exafore_planning/issues/681 */
#define GLO_MAX_STEP_NUM 30

#define SIN_5 0.0871557427476582
#define COS_5 0.9961946980917456

#define EPHEMERIS_NULL_LOG_MESSAGE "null ephemeris"

#define EPHEMERIS_INVALID_LOG_MESSAGE \
  "%s ephemeris (v:%d, fi:%d, [%d, %f]), [%d, %f]"

#define EPHEMERIS_INVALID_IOD_LOG_MESSAGE \
  "invalid IOD ephemeris (v:%d, fi:%d, [%d, %f], iodc:%d, iode:%d), [%d, %f]"

/* Galileo OS SIS ICD, Table 71 */
enum gal_data_validity_status_t {
  GAL_DVS_NAVIGATION_DATA_VALID,
  GAL_DVS_WORKING_WITHOUT_GUARANTEE
};

/* Galileo OS SIS ICD, Table 74 */
enum gal_health_status_t {
  GAL_HS_SIGNAL_OK,
  GAL_HS_SIGNAL_OUT_OF_SERVICE,
  GAL_HS_SIGNAL_WILL_BE_OUT_OF_SERVICE,
  GAL_HS_SIGNAL_COMPONENT_IN_TEST
};

/**
 * Beidou URA table.
 * Reference: BDS-SIS-ICD-2.1
 */
static const float bds_ura_table[15] = {[0] = 2.0f,
                                        [1] = 2.8f,
                                        [2] = 4.0f,
                                        [3] = 5.7f,
                                        [4] = 8.0f,
                                        [5] = 11.3f,
                                        [6] = 16.0f,
                                        [7] = 32.0f,
                                        [8] = 64.0f,
                                        [9] = 128.0f,
                                        [10] = 256.0f,
                                        [11] = 512.0f,
                                        [12] = 1024.0f,
                                        [13] = 2048.0f,
                                        [14] = 4096.0f};

/**
 * Helper to sign extend 14-bit value
 *
 * \param[in] arg Unsigned integer
 *
 * \return Sign-extended integer
 */
static inline s32 sign_extend14(u32 arg) {
  return BITS_SIGN_EXTEND_32(14, arg);
}

/**
 * Helper to sign extend 22-bit value
 *
 * \param[in] arg Unsigned integer
 *
 * \return Sign-extended integer
 */
static inline s32 sign_extend22(u32 arg) {
  return BITS_SIGN_EXTEND_32(22, arg);
}

/**
 * Helper to sign extend 24-bit value

 * \param[in] arg Unsigned integer
 *
 * \return Sign-extended integer
 */
static inline s32 sign_extend24(u32 arg) {
  return BITS_SIGN_EXTEND_32(24, arg);
}

/**
 * \page tot_note Regarding the time used to compute satellite state.
 *
 * The following calc_sat_state* methods take a time of transmission, tot,
 * which should be in satellite time (the time of transmission
 * according to the satellite).  In otherwords, this time MUST include satellite
 * clock error! So, for example,
 *
 *   tot = time_received - pseudorange / C
 *
 * is a valid way of determining tot. This is because the pseudorange is
 * defined as:
 *
 *   pseudorange = (time_received_k - tot^s) * C
 *               = ((tor_gps + dt_k) - (tot_gps + dt^s)) * C
 *
 * where _k indicates a receiver specific term and ^s indicates a satellite
 * specific term and _gps indicates GPS system time and dt represents a clock
 * error. Notice that using this notation we get:
 *
 *   tot = tot_gps + dt^s
 *
 * inside these calc_sat_state* methods we will first compute the satellite
 * clock error (dt^s), remove it from the time of transmission to get the
 * time of transmission in GPS system time, then compute the satellite position.
 *
 * As a result inferring the time of transmission from geometry will require
 * iteration. Using pseudocode (these functions don't exist) that would be:
 *
 *   tot_gps = tor_gps - range / C
 *   sat_clock_error = calc_sat_clock(tot_gps)
 *   tot = tot_gps + sat_clock_error
 *   sat_po = calc_sat_pos(tot_gps, ...)
 */

/** Calculate satellite position, velocity and clock offset from SBAS ephemeris.
 *
 * References:
 *   -# WAAS Specification FAA-E-2892b 4.4.11
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 *
 * \return  0 on success,
 *         -1 if ephemeris is not valid or too old
 */
static s8 calc_sat_state_xyz(const ephemeris_t *e,
                             const gps_time_t *t,
                             double pos[3],
                             double vel[3],
                             double acc[3],
                             double *clock_err,
                             double *clock_rate_err) {
  /* TODO should t be in GPS or SBAS time? */
  /* TODO what is the SBAS valid ttime interval? */

  const ephemeris_xyz_t *ex = &e->data.xyz;

  double dt = gpsdifftime(t, &e->toe);

  *clock_err = ex->a_gf0 + dt * ex->a_gf1;
  *clock_rate_err = ex->a_gf1;

  dt -= *clock_err;

  vel[0] = ex->vel[0] + ex->acc[0] * dt;
  vel[1] = ex->vel[1] + ex->acc[1] * dt;
  vel[2] = ex->vel[2] + ex->acc[2] * dt;

  double dt2 = dt * dt;

  pos[0] = ex->pos[0] + ex->vel[0] * dt + 0.5 * ex->acc[0] * dt2;
  pos[1] = ex->pos[1] + ex->vel[1] * dt + 0.5 * ex->acc[1] * dt2;
  pos[2] = ex->pos[2] + ex->vel[2] * dt + 0.5 * ex->acc[2] * dt2;

  acc[0] = ex->acc[0];
  acc[1] = ex->acc[1];
  acc[2] = ex->acc[2];

  return 0;
}

/** Re-calculation of the acceleration in ECEF using
 *  a position and velocity (ECEF) and acceleration term (ECI)
 *
 *  ICD 5.1: A.3.1.2, with corrections from RTCM 3.2 p.186
 *
 * \param vel_acc Pointer to concatenation of velocities and accelerations
 * (ECEF)
 * \param pos Pointer to position input array (ECEF)
 * \param vel Pointer to velocity input array (ECEF)
 * \param acc Pointer to acceleration input array (ECI)
 */
static void calc_ecef_vel_acc(double vel_acc[6],
                              const double pos[3],
                              const double vel[3],
                              const double acc[3]) {
  double r = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);

  double m_r3 = GLO_GM / (r * r * r);
  double inv_r2 = 1 / (r * r);

  double g_term = 3.0 / 2.0 * GLO_J02 * m_r3 * GLO_A_E * GLO_A_E * inv_r2;

  double lg_term = (1.0 - 5.0 * pos[2] * pos[2] * inv_r2);

  double omega_sqr = GLO_OMEGAE_DOT * GLO_OMEGAE_DOT;

  vel_acc[0] = vel[0];
  vel_acc[1] = vel[1];
  vel_acc[2] = vel[2];

  vel_acc[3] = -m_r3 * pos[0] - g_term * pos[0] * lg_term + omega_sqr * pos[0] +
               2.0 * GLO_OMEGAE_DOT * vel[1] + acc[0];

  vel_acc[4] = -m_r3 * pos[1] - g_term * pos[1] * lg_term + omega_sqr * pos[1] -
               2.0 * GLO_OMEGAE_DOT * vel[0] + acc[1];

  vel_acc[5] = -m_r3 * pos[2] - g_term * pos[2] * (2.0 + lg_term) + acc[2];
}

/** Calculate satellite position, velocity and clock offset from GLO ephemeris.
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 *
 */
static s8 calc_sat_state_glo(const ephemeris_t *e,
                             const gps_time_t *t,
                             double pos[3],
                             double vel[3],
                             double acc[3],
                             double *clock_err,
                             double *clock_rate_err) {
  assert(e != NULL);
  assert(t != NULL);
  assert(pos != NULL);
  assert(vel != NULL);
  assert(acc != NULL);
  assert(clock_err != NULL);
  assert(clock_rate_err != NULL);

  /* NOTE: toe should be in GPS time as well */
  double dt = gpsdifftime(t, &e->toe);

  float tgd;
  if (get_tgd_correction(e, &e->sid, &tgd) != 0) {
    return -1;
  }

  *clock_err = -e->data.glo.tau + e->data.glo.gamma * dt - tgd;
  *clock_rate_err = e->data.glo.gamma;

  dt -= *clock_err;

  u32 num_steps = (u32)ceil(fabs(dt) / GLO_MAX_STEP_LENGTH);
  num_steps = MIN(num_steps, GLO_MAX_STEP_NUM);

  double ecef_vel_acc[6];
  if (num_steps) {
    double h = dt / num_steps;

    double y[6];

    calc_ecef_vel_acc(
        ecef_vel_acc, e->data.glo.pos, e->data.glo.vel, e->data.glo.acc);
    memcpy(&y[0], e->data.glo.pos, sizeof(double) * 3);
    memcpy(&y[3], e->data.glo.vel, sizeof(double) * 3);

    /* Runge-Kutta integration algorithm */
    for (u32 i = 0; i < num_steps; i++) {
      double k1[6], k2[6], k3[6], k4[6], y_tmp[6];
      u8 j;

      memcpy(k1, ecef_vel_acc, sizeof(k1));

      for (j = 0; j < 6; j++) {
        y_tmp[j] = y[j] + h / 2 * k1[j];
      }

      calc_ecef_vel_acc(k2, &y_tmp[0], &y_tmp[3], e->data.glo.acc);

      for (j = 0; j < 6; j++) {
        y_tmp[j] = y[j] + h / 2 * k2[j];
      }

      calc_ecef_vel_acc(k3, &y_tmp[0], &y_tmp[3], e->data.glo.acc);

      for (j = 0; j < 6; j++) {
        y_tmp[j] = y[j] + h * k3[j];
      }

      calc_ecef_vel_acc(k4, &y_tmp[0], &y_tmp[3], e->data.glo.acc);

      for (j = 0; j < 6; j++) {
        y[j] += h / 6 * (k1[j] + 2 * k2[j] + 2 * k3[j] + k4[j]);
      }

      calc_ecef_vel_acc(ecef_vel_acc, &y[0], &y[3], e->data.glo.acc);
    }
    memcpy(pos, &y[0], sizeof(double) * 3);
    memcpy(vel, &y[3], sizeof(double) * 3);
  } else {
    memcpy(pos, e->data.glo.pos, sizeof(double) * 3);
    memcpy(vel, e->data.glo.vel, sizeof(double) * 3);
  }
  /* Here we compute the final acceleration (ECEF). */
  calc_ecef_vel_acc(ecef_vel_acc, pos, vel, e->data.glo.acc);
  memcpy(acc, &ecef_vel_acc[3], sizeof(double) * 3);

  return 0;
}

/** Calculate satellite position, velocity and clock offset from GPS ephemeris.
 *
 * References:
 *   -# IS-GPS-200D, Section 20.3.3.3.3.1 and Table 20-IV
 *   -http://math.tut.fi/posgroup/korvenoja_piche_ion2000a.pdf which
 *    was used to implement the acceleration terms, note however there are
 *    several typos.  In particular in the equation for z_s'' the inc ** 2
 *    term should be inc' ** 2.  And for x_s'' the omega' x_p term should
 *    be omega' x_p'.  Each of these can be confirmed by checking units.
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param orbit_type satellite orbit type [unitless]
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 *
 * \return  0 on success,
 *         -1 if ephemeris is not valid or too old
 */
static s8 calc_sat_state_kepler(const ephemeris_t *e,
                                const gps_time_t *t,
                                const satellite_orbit_type_t orbit_type,
                                double pos[3],
                                double vel[3],
                                double acc[3],
                                double *clock_err,
                                double *clock_rate_err) {
  const ephemeris_kepler_t *k = &e->data.kepler;

  /* Calculate satellite clock terms */

  /* Seconds from clock data reference time (toc) */
  double dt = gpsdifftime(t, &k->toc);

  float tgd;
  if (get_tgd_correction(e, &e->sid, &tgd) != 0) {
    return -1;
  }

  /* According to the GPS ICD the satellite clock is reported
   * in iono free form, which means that the clock errors for
   * L1 and L2 need to take the group delay into account */
  *clock_err = k->af0 + dt * (k->af1 + dt * k->af2) - tgd;
  *clock_rate_err = k->af1 + 2.0 * dt * k->af2;

  /* Seconds from the time from ephemeris reference epoch (toe) */
  dt = gpsdifftime(t, &e->toe) - *clock_err;

  /* Gravitational Constant */
  double gm;
  switch (sid_to_constellation(e->sid)) {
    case CONSTELLATION_GPS:
    case CONSTELLATION_QZS:
      gm = GPS_GM;
      break;
    case CONSTELLATION_BDS:
      gm = BDS2_GM;
      break;
    case CONSTELLATION_GAL:
      gm = GAL_GM;
      break;
    case CONSTELLATION_SBAS:
    case CONSTELLATION_GLO:
    case CONSTELLATION_COUNT:
    case CONSTELLATION_INVALID:
    default:
      log_error("Unknown constellation %d in Keplerian ephemeris computation",
                sid_to_constellation(e->sid));
      return -1;
  }

  /* Calculate position per IS-GPS-200D p 97 Table 20-IV */

  /* Semi-major axis in meters. */
  double a = k->sqrta * k->sqrta;
  /* Corrected mean motion in radians/sec. */
  double ma_dot = sqrt(gm / (a * a * a)) + k->dn;
  /* Corrected mean anomaly in radians. */
  double ma = k->m0 + ma_dot * dt;

  /* Iteratively solve for the Eccentric Anomaly
   * (from Keith Alter and David Johnston) */
  double ea = ma; /* Starting value for E. */
  double ea_old;
  double temp;
  double ecc = k->ecc;
  u8 count = 0;

  /* TODO: Implement convergence test using integer difference of doubles,
   * http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
   */
  do {
    ea_old = ea;
    temp = 1.0 - ecc * cos(ea_old);
    ea = ea + (ma - ea_old + ecc * sin(ea_old)) / temp;
    count++;
    if (count > 5) {
      break;
    }
  } while (fabs(ea - ea_old) > 1.0E-14);

  double ea_dot = ma_dot / temp;
  double ea_acc = ea_dot * ea_dot * ecc * sin(ea) / temp;

  /* Begin calc for True Anomaly and Argument of Latitude */
  double temp2 = sqrt(1.0 - ecc * ecc);
  /* Argument of Latitude = True Anomaly + Argument of Perigee. */
  double al = atan2(temp2 * sin(ea), cos(ea) - ecc) + k->w;
  double al_dot = temp2 * ea_dot / temp;
  double al_acc = 2 * al_dot * ea_acc / ea_dot;
  double al_dot_sqr = al_dot * al_dot;

  /* These values are used all over the place, only do them once. */
  double sin2al = sin(2.0 * al);
  double cos2al = cos(2.0 * al);

  /* Calculate the argument of latitude correction */
  double dal = k->cus * sin2al + k->cuc * cos2al;
  double dal_dot = 2 * al_dot * (k->cus * cos2al - k->cuc * sin2al);
  double dal_acc = -4 * al_dot_sqr * dal + al_acc / al_dot * dal_dot;

  /* Calculate corrected argument of latitude based on position. */
  double cal = al + dal;
  double cal_dot = al_dot + dal_dot;
  double cal_acc = al_acc + dal_acc;

  /* Calculate the radius correction */
  double dr = (k->crs * sin2al + k->crc * cos2al);
  double dr_dot = 2 * al_dot * (k->crs * cos2al - k->crc * sin2al);
  double dr_acc = 4 * al_dot_sqr * dr + al_acc / al_dot * dr_dot;

  /* Calculate corrected radius based on argument of latitude. */
  double r = a * temp + k->crc * cos2al + k->crs * sin2al;
  double r_dot = a * ecc * sin(ea) * ea_dot +
                 2.0 * al_dot * (k->crs * cos2al - k->crc * sin2al);
  double r_acc =
      a * ecc * ea_dot * ea_dot * cos(ea) + a * ecc * ea_acc * sin(ea) + dr_acc;

  /* Relativistic correction term to satellite clock using x.v = r . r_dot. */
  double einstein = -2.0 * r * r_dot / GPS_C / GPS_C;
  *clock_err += einstein;

  /* Calculate the inclination correction */
  double dinc = (k->cis * sin2al + k->cic * cos2al);
  double dinc_dot = 2 * al_dot * (k->cis * cos2al - k->cic * sin2al);
  double dinc_acc = -4 * al_dot_sqr * dinc + al_acc / al_dot * dinc_dot;

  /* Calculate inclination based on argument of latitude. */
  double inc = k->inc + k->inc_dot * dt + k->cic * cos2al + k->cis * sin2al;
  double inc_dot =
      k->inc_dot + 2.0 * al_dot * (k->cis * cos2al - k->cic * sin2al);
  double inc_acc = dinc_acc;

  /* Calculate position and velocity in orbital plane. */
  double x = r * cos(cal);
  double y = r * sin(cal);
  double x_dot = r_dot * cos(cal) - y * cal_dot;
  double y_dot = r_dot * sin(cal) + x * cal_dot;
  double cal_dot_sqr = cal_dot * cal_dot;
  double x_acc = -cal_dot_sqr * x - cal_acc * y -
                 2 * cal_dot * r_dot * sin(cal) + r_acc * cos(cal);
  double y_acc = -cal_dot_sqr * y + cal_acc * x +
                 2 * cal_dot * r_dot * cos(cal) + r_acc * sin(cal);

  /* Corrected longitude of ascenting node. */
  double om_dot;
  double om;
  switch (sid_to_constellation(e->sid)) {
    case CONSTELLATION_GPS:
    case CONSTELLATION_QZS:
    case CONSTELLATION_GAL:
      om_dot = k->omegadot - GPS_OMEGAE_DOT;
      om = k->omega0 + dt * om_dot - GPS_OMEGAE_DOT * e->toe.tow;
      break;
    case CONSTELLATION_BDS:
      if (orbit_type == GEO) {
        om_dot = k->omegadot;
      } else {
        om_dot = k->omegadot - BDS2_OMEGAE_DOT;
      }
      om = k->omega0 + dt * om_dot -
           BDS2_OMEGAE_DOT * (e->toe.tow - BDS_SECOND_TO_GPS_SECOND);
      break;
    case CONSTELLATION_SBAS:
    case CONSTELLATION_GLO:
    case CONSTELLATION_COUNT:
    case CONSTELLATION_INVALID:
    default:
      log_error("Unknown constellation %d in Keplerian ephemeris computation",
                sid_to_constellation(e->sid));
      return -1;
  }

  /* Compute the satellite's position in Earth-Centered Earth-Fixed
   * coordiates. */
  pos[0] = x * cos(om) - y * cos(inc) * sin(om);
  pos[1] = x * sin(om) + y * cos(inc) * cos(om);
  pos[2] = y * sin(inc);

  /* transform GEO user-defined inertial system to BDCS, similar applies to
   * velocity and acceleration */
  if (orbit_type == GEO) {
    double pos_bds[3];
    double sin_ome = sin(BDS2_OMEGAE_DOT * dt);
    double cos_ome = cos(BDS2_OMEGAE_DOT * dt);
    pos_bds[0] = pos[0] * cos_ome + pos[1] * sin_ome * COS_5 +
                 pos[2] * sin_ome * (-SIN_5);
    pos_bds[1] = -pos[0] * sin_ome + pos[1] * cos_ome * COS_5 +
                 pos[2] * cos_ome * (-SIN_5);
    pos_bds[2] = -pos[1] * (-SIN_5) + pos[2] * COS_5;

    pos[0] = pos_bds[0];
    pos[1] = pos_bds[1];
    pos[2] = pos_bds[2];
  }

  /* Compute the satellite's velocity in Earth-Centered Earth-Fixed
   * coordiates. */
  temp = y_dot * cos(inc) - y * sin(inc) * inc_dot;
  vel[0] = -om_dot * pos[1] + x_dot * cos(om) - temp * sin(om);
  vel[1] = om_dot * pos[0] + x_dot * sin(om) + temp * cos(om);
  vel[2] = y * cos(inc) * inc_dot + y_dot * sin(inc);

  /* Note that there is a typo in the reference used for this equation.
     The reference uses  omega' * x which should be  omeaga' * x'. */
  double acc_common_1 = vel[2] * inc_dot - om_dot * x_dot +
                        y * inc_acc * sin(inc) - y_acc * cos(inc) +
                        inc_dot * y_dot * sin(inc);
  double acc_common_2 =
      x_acc + y * om_dot * inc_dot * sin(inc) - om_dot * y_dot * cos(inc);
  acc[0] = -om_dot * vel[1] + sin(om) * acc_common_1 + cos(om) * acc_common_2;
  acc[1] = om_dot * vel[0] - cos(om) * acc_common_1 + sin(om) * acc_common_2;
  /* Note that there is a typo in the reference used for this equation.
     The reference uses -y * inc^2 which should be -y * inc'^2. */
  acc[2] = sin(inc) * (-y * inc_dot * inc_dot + y_acc) +
           cos(inc) * (y * inc_acc + 2 * inc_dot * y_dot);

  return 0;
}

/** Calculate satellite position, velocity and clock offset from ephemeris.
 *
 * Dispatch to internal function for Kepler/XYZ ephemeris depending on
 * constellation.
 *
 * Assuming a MEO satellite
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 * \param iodc Issue of data clock [unitless]
 * \param iode Issue of data ephemeris [unitless]
 *
 * \return  0 on success,
 *         -1 if ephemeris is invalid
 */
s8 calc_sat_state(const ephemeris_t *e,
                  const gps_time_t *t,
                  double pos[3],
                  double vel[3],
                  double acc[3],
                  double *clock_err,
                  double *clock_rate_err) {
  const satellite_orbit_type_t orbit_type = MEO;

  return calc_sat_state_orbit_type(
      e, t, orbit_type, pos, vel, acc, clock_err, clock_rate_err);
}

/** Calculate satellite position, velocity and clock offset from ephemeris
 *  considering the satellite orbit type.
 *
 * Dispatch to internal function for Kepler/XYZ ephemeris depending on
 * constellation.
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param orbit_type satellite orbit type [unitless]
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 * \param iodc Issue of data clock [unitless]
 * \param iode Issue of data ephemeris [unitless]
 *
 * \return  0 on success,
 *         -1 if ephemeris is invalid
 */
s8 calc_sat_state_orbit_type(const ephemeris_t *e,
                             const gps_time_t *t,
                             const satellite_orbit_type_t orbit_type,
                             double pos[3],
                             double vel[3],
                             double acc[3],
                             double *clock_err,
                             double *clock_rate_err) {
  assert(pos != NULL);
  assert(vel != NULL);
  assert(clock_err != NULL);
  assert(clock_rate_err != NULL);
  assert(e != NULL);

  const ephemeris_status_t eph_status = ephemeris_valid_detailed(e, t);
  if (eph_status != EPH_VALID) {
    return -1;
  }

  return calc_sat_state_n(
      e, t, orbit_type, pos, vel, acc, clock_err, clock_rate_err);
}

/** Calculate satellite position, velocity and clock offset from ephemeris
 * without ephemeris validity check.
 *
 * Dispatch to internal function for Kepler/XYZ ephemeris depending on
 * constellation.
 *
 * \param e Pointer to an ephemeris structure for the satellite of interest
 * \param t Satellite time at which to calculate the state (See \ref tot_note)
 * \param orbit_type satellite orbit type [unitless]
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 * \param iodc Issue of data clock [unitless]
 * \param iode Issue of data ephemeris [unitless]
 *
 * \return  0 on success,
 *         -1 if ephemeris is invalid
 */
s8 calc_sat_state_n(const ephemeris_t *e,
                    const gps_time_t *t,
                    const satellite_orbit_type_t orbit_type,
                    double pos[3],
                    double vel[3],
                    double acc[3],
                    double *clock_err,
                    double *clock_rate_err) {
  assert(pos != NULL);
  assert(vel != NULL);
  assert(clock_err != NULL);
  assert(clock_rate_err != NULL);
  assert(e != NULL);

  switch (sid_to_constellation(e->sid)) {
    case CONSTELLATION_GPS:
    case CONSTELLATION_BDS:
    case CONSTELLATION_GAL:
    case CONSTELLATION_QZS:
      return calc_sat_state_kepler(
          e, t, orbit_type, pos, vel, acc, clock_err, clock_rate_err);
    case CONSTELLATION_SBAS:
      return calc_sat_state_xyz(e, t, pos, vel, acc, clock_err, clock_rate_err);
    case CONSTELLATION_GLO:
      return calc_sat_state_glo(e, t, pos, vel, acc, clock_err, clock_rate_err);
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      assert(!"Unsupported constellation");
      return -1;
  }
}

/** Calculate the azimuth and elevation of a satellite from a reference
 * position given the satellite ephemeris.
 *
 * \param e  Pointer to an ephemeris structure for the satellite of interest.
 * \param t  Satellite time at which to calculate the az/el (See \ref tot_note)
 * \param ref  ECEF coordinates of the reference point from which the azimuth
 *             and elevation is to be determined, passed as [X, Y, Z], all in
 *             meters.
 * \param orbit_type    satellite orbit type [unitless]
 * \param az   Pointer to where to store the calculated azimuth output [rad].
 * \param el   Pointer to where to store the calculated elevation output [rad].
 * \param check_e set this parameter as "true" if ephemeris validity check
 *                needed, otherwise set as "false"
 * \return  0 on success,
 *         -1 if almanac is not valid or too old
 */
s8 calc_sat_az_el(const ephemeris_t *e,
                  const gps_time_t *t,
                  const double ref[3],
                  const satellite_orbit_type_t orbit_type,
                  double *az,
                  double *el,
                  bool check_e) {
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err, clock_rate_err;
  s8 ret;
  if (check_e) {
    ret = calc_sat_state_orbit_type(e,
                                    t,
                                    orbit_type,
                                    sat_pos,
                                    sat_vel,
                                    sat_acc,
                                    &clock_err,
                                    &clock_rate_err);
  } else {
    ret = calc_sat_state_n(e,
                           t,
                           orbit_type,
                           sat_pos,
                           sat_vel,
                           sat_acc,
                           &clock_err,
                           &clock_rate_err);
  }
  if (ret != 0) {
    return ret;
  }
  wgsecef2azel(sat_pos, ref, az, el);

  return 0;
}

/** Calculate the Doppler shift of a satellite as observed at a reference
 * position given the satellite ephemeris.
 *
 * \param e  Pointer to an ephemeris structure for the satellite of interest.
 * \param t  Satellite time at which to calculate doppler (See \ref tot_note)
 * \param ref_pos  ECEF coordinates of the reference point from which the
 *                 Doppler is to be determined, passed as [X, Y, Z], all in
 *                 meters.
 * \param ref_vel ECEF speed vector of the receiver, m/s.
 * \param orbit_type satellite orbit type [unitless]
 * \param doppler The Doppler shift [Hz].
 * \return  0 on success,
 *         -1 if ephemeris is not valid or too old
 */
s8 calc_sat_doppler(const ephemeris_t *e,
                    const gps_time_t *t,
                    const double ref_pos[3],
                    const double ref_vel[3],
                    const satellite_orbit_type_t orbit_type,
                    double *doppler) {
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err, clock_rate_err;
  double vec_ref_sat_pos[3];
  double vec_ref_sat_vel[3];

  s8 ret = calc_sat_state_orbit_type(
      e, t, orbit_type, sat_pos, sat_vel, sat_acc, &clock_err, &clock_rate_err);
  if (ret != 0) {
    return ret;
  }

  /* Find the vector from the reference position to the satellite. */
  vector_subtract(3, sat_pos, ref_pos, vec_ref_sat_pos);

  /* Find the velocity diff between receiver and satellite. */
  vector_add(3, sat_vel, ref_vel, vec_ref_sat_vel);

  /* Find the satellite - receiver velocity projected on the line of sight
   * vector from the reference position to the satellite. */
  double radial_vel = vector_dot(3, vec_ref_sat_pos, vec_ref_sat_vel) /
                      vector_norm(3, vec_ref_sat_pos);

  /* Return the Doppler shift. */
  *doppler = sid_to_carr_freq(e->sid) * radial_vel / GPS_C;

  return 0;
}

/* Fake GPS week numbers.
   Make sure that both week numbers are set.
   The result will be as if the week numbers had been chosen for the times
   to be as close as possible. */
static void fake_gps_wns(gps_time_t *t1, gps_time_t *t2) {
  assert(t1);
  assert((int)t1->tow != TOW_UNKNOWN);
  assert((WN_UNKNOWN == t1->wn) || (t1->wn >= 0));

  assert(t2);
  assert((int)t2->tow != TOW_UNKNOWN);
  assert((WN_UNKNOWN == t2->wn) || (t2->wn >= 0));

  if ((WN_UNKNOWN != t1->wn) && (WN_UNKNOWN != t2->wn)) {
    return; /* nothing to fix */
  }

  if ((WN_UNKNOWN == t1->wn) && (WN_UNKNOWN == t2->wn)) {
    /* This is arbitrary. Has no significance other than being more than 1. */
    t1->wn = 2;
  }
  double dt_s = t1->tow - t2->tow;
  if (dt_s > (WEEK_SECS / 2)) {
    if (WN_UNKNOWN == t1->wn) {
      t1->wn = MAX(t2->wn - 1, 0);
    } else {
      t2->wn = t1->wn + 1;
    }
  } else if (dt_s < (-WEEK_SECS / 2)) {
    if (WN_UNKNOWN == t1->wn) {
      t1->wn = t2->wn + 1;
    } else {
      t2->wn = MAX(t1->wn - 1, 0);
    }
  } else if (WN_UNKNOWN == t1->wn) {
    t1->wn = t2->wn;
  } else {
    assert(WN_UNKNOWN == t2->wn);
    t2->wn = t1->wn;
  }
  assert(
      fabs(gpsdifftime(t1, t2)) <= (WEEK_SECS / 2) ||
      ((t1->wn == 0 || t2->wn == 0) && fabs(gpsdifftime(t1, t2)) <= WEEK_SECS));
}

/** Gets the status of an ephemeris - is the ephemeris invalid, unhealthy, or
 * has some other condition which makes it unusable?
 *
 * \param e Ephemeris struct
 * \return ephemeris_status_t The status of the ephemeris e.
 */
ephemeris_status_t get_ephemeris_status_t(const ephemeris_t *e) {
  if (e == NULL) {
    return EPH_NULL;
  }
  if (!e->valid) {
    return EPH_INVALID;
  }
  if (e->toe.wn == 0) {
    return EPH_WN_EQ_0; /* ephemeris did not get timestamped when it was
                           received */
  }
  if (0 == e->fit_interval) {
    return EPH_FIT_INTERVAL_EQ_0;
  }
  if (!ephemeris_healthy(e, e->sid.code)) {
    return EPH_UNHEALTHY;
  }
  switch (sid_to_constellation(e->sid)) {
    case CONSTELLATION_GPS:
    case CONSTELLATION_QZS:
      if (e->data.kepler.iodc > GPS_IODC_MAX ||
          e->data.kepler.iode > GPS_IODE_MAX ||
          (e->data.kepler.iodc & GPS_IODE_MAX) !=
              (e->data.kepler.iode & GPS_IODE_MAX)) {
        return EPH_INVALID_IOD;
      }
      break;
    case CONSTELLATION_BDS:
      if (e->source == EPH_SOURCE_BDS_D1_D2_NAV) {
        if (e->data.kepler.iodc > BDS2_IODC_MAX ||
            e->data.kepler.iode > BDS2_IODE_MAX) {
          return EPH_INVALID_IOD;
        }
      } else {
        if (e->data.kepler.iodc > BDS3_IODC_MAX ||
            e->data.kepler.iode > BDS3_IODE_MAX ||
            (e->data.kepler.iodc & BDS3_IODE_MAX) !=
                (e->data.kepler.iode & BDS3_IODE_MAX)) {
          return EPH_INVALID_IOD;
        }
      }
      break;
    case CONSTELLATION_GAL:
      if (e->data.kepler.iodc > GAL_IOD_NAV_MAX ||
          e->data.kepler.iode > GAL_IOD_NAV_MAX ||
          e->data.kepler.iodc != e->data.kepler.iode) {
        return EPH_INVALID_IOD;
      }
      break;
    case CONSTELLATION_SBAS:
      break;
    case CONSTELLATION_GLO:
      if (e->data.glo.iod > GLO_IOD_MAX) {
        return EPH_INVALID_IOD;
      }
      break;
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      return EPH_INVALID_SID;
  }
  return EPH_VALID;
}

/** Used internally by other ephemeris valid functions. Given a valid ephemeris,
 * is this ephemeris valid at gps time t?
 *
 * \param e Ephemeris struct
 * \param t The current GPS time. This is used to determine the ephemeris age.
 * \return 1 if the ephemeris is valid and not too old.
 *         0 otherwise.
 */
static u8 ephemeris_valid_at_time(const ephemeris_t *e, const gps_time_t *t) {
  gps_time_t toe = e->toe;
  gps_time_t tm = *t;
  fake_gps_wns(&toe, &tm);

  gps_time_t bgn = toe;
  gps_time_t end = toe;

  const gps_time_t *toc = NULL;
  if (IS_GPS(e->sid) || IS_QZSS(e->sid)) {
    /* TOE is a middle of ephemeris validity interval */
    bgn.tow -= e->fit_interval / 2;
    end.tow += e->fit_interval / 2;
    toc = &e->data.kepler.toc;
  } else if (IS_BDS2(e->sid) || IS_GAL(e->sid)) {
    /* TOE is the beginning of ephemeris validity interval */
    end.tow += e->fit_interval;
    toc = &e->data.kepler.toc;
  } else if (IS_GLO(e->sid)) {
    /* TOE is a middle of ephemeris validity interval */
    bgn.tow -= e->fit_interval / 2;
    end.tow += e->fit_interval / 2;
  } else if (IS_SBAS(e->sid)) {
    return 1;
  } else {
    assert(0);
    return 0;
  }
  normalize_gps_time(&bgn);
  normalize_gps_time(&end);

  return ephemeris_params_valid(&bgn, &end, toc, &tm);
}

/** Is this ephemeris usable?
 *
 * \param e Ephemeris struct
 * \param t The current GPS time. This is used to determine the ephemeris age.
 * \return 1 if the ephemeris is valid and not too old.
 *         0 otherwise.
 */
u8 ephemeris_valid(const ephemeris_t *e, const gps_time_t *t) {
  if (t == NULL) {
    return 0;
  }

  const ephemeris_status_t eph_status = get_ephemeris_status_t(e);

  if (eph_status != EPH_VALID) {
    return 0;
  }

  return ephemeris_valid_at_time(e, t);
}

/** Is this ephemeris usable? Similar to `ephemeris_valid()`, but returns an
 * `ephemeris_status_t` as well as logs if there's an unexpected ephemeris
 * status.
 *
 * \param e Ephemeris struct
 * \param t The current GPS time. This is used to determine the ephemeris age.
 * \return ephemeris_status_t The status of the ephemeris e at gps_time t.
 */
ephemeris_status_t ephemeris_valid_detailed(const ephemeris_t *e,
                                            const gps_time_t *t) {
  if (t == NULL) {
    assert(false);
  }

  ephemeris_status_t eph_status = get_ephemeris_status_t(e);

  switch (eph_status) {
    case EPH_NULL:
      log_error(EPHEMERIS_NULL_LOG_MESSAGE);
      break;
    case EPH_INVALID:
      log_info_sid(e->sid,
                   EPHEMERIS_INVALID_LOG_MESSAGE,
                   "invalid",
                   (int)e->valid,
                   (int)e->fit_interval,
                   (int)e->toe.wn,
                   e->toe.tow,
                   (int)t->wn,
                   t->tow);
      break;
    case EPH_WN_EQ_0:
      log_error_sid(e->sid,
                    EPHEMERIS_INVALID_LOG_MESSAGE,
                    "wn == 0",
                    (int)e->valid,
                    (int)e->fit_interval,
                    (int)e->toe.wn,
                    e->toe.tow,
                    (int)t->wn,
                    t->tow);
      break;
    case EPH_FIT_INTERVAL_EQ_0:
      log_error_sid(e->sid,
                    EPHEMERIS_INVALID_LOG_MESSAGE,
                    "fit_interval == 0",
                    (int)e->valid,
                    (int)e->fit_interval,
                    (int)e->toe.wn,
                    e->toe.tow,
                    (int)t->wn,
                    t->tow);
      break;
    case EPH_UNHEALTHY:
      log_info_sid(e->sid,
                   EPHEMERIS_INVALID_LOG_MESSAGE,
                   "unhealthy",
                   (int)e->valid,
                   (int)e->fit_interval,
                   (int)e->toe.wn,
                   e->toe.tow,
                   (int)t->wn,
                   t->tow);
      break;
    case EPH_INVALID_SID:
      log_info_sid(e->sid,
                   EPHEMERIS_INVALID_LOG_MESSAGE,
                   "sid invalid",
                   (int)e->valid,
                   (int)e->fit_interval,
                   (int)e->toe.wn,
                   e->toe.tow,
                   (int)t->wn,
                   t->tow);
      break;
    case EPH_INVALID_IOD:
      log_info_sid(e->sid,
                   EPHEMERIS_INVALID_IOD_LOG_MESSAGE,
                   (int)e->valid,
                   (int)e->fit_interval,
                   (int)e->toe.wn,
                   e->toe.tow,
                   e->data.kepler.iodc,
                   e->data.kepler.iode,
                   (int)t->wn,
                   t->tow);
      break;
    case EPH_TOO_OLD:
    case EPH_VALID:
    default:
      break;
  }

  if (eph_status != EPH_VALID) {
    return eph_status;
  }

  if (0 == ephemeris_valid_at_time(e, t)) {
    eph_status = EPH_TOO_OLD;
  }

  return eph_status;
}

/** Lean version of ephemeris_valid
 * The function allows to avoid passing whole ephemeris
 *
 * \param bgn ephemeris validity interval begins at this GPS time
 * \param end ephemeris validity interval ends at this GPS time
 * \param toc Time from ephemeris clock reference epoch. Can be NULL.
 * \param t The current GPS time. This is used to determine the ephemeris age
 * \return 1 if the ephemeris is valid and not too old.
 *         0 otherwise.
 */
u8 ephemeris_params_valid(const gps_time_t *bgn,
                          const gps_time_t *end,
                          const gps_time_t *toc,
                          const gps_time_t *t) {
  assert(t != NULL);
  assert(bgn);
  assert(end);

  /* TODO: this doesn't exclude ephemerides older than a week so could be made
   * better. */
  if (!gpstime_in_range(bgn, end, t)) {
    return 0;
  }

  if (toc != NULL) {
    if (toc->wn == 0) {
      return 0;
    }

    if (!gpstime_in_range(bgn, end, toc)) {
      return 0;
    }
  }

  return 1;
}

#define URA_VALUE_TABLE_LEN 16

static const float gps_ura_values[URA_VALUE_TABLE_LEN] = {
    [0] = 2.0f,
    [1] = 2.8f,
    [2] = 4.0f,
    [3] = 5.7f,
    [4] = 8.0f,
    [5] = 11.3f,
    [6] = 16.0f,
    [7] = 32.0f,
    [8] = 64.0f,
    [9] = 128.0f,
    [10] = 256.0f,
    [11] = 512.0f,
    [12] = 1024.0f,
    [13] = 2048.0f,
    [14] = 4096.0f,
    [15] = 6144.0f,
};

/** Convert a GPS URA index into a URA value.
 *
 * \param index URA index.
 * \return the URA in meters.
 */
float decode_ura_index(const u8 index) {
  /* Invalid index */
  if (URA_VALUE_TABLE_LEN < index) {
    return INVALID_URA_VALUE;
  }

  return gps_ura_values[index];
}

/** Convert GPS URA into URA index.
 *
 * \param ura URA in meters.
 * \return URA index.
 */
u8 encode_ura(float ura) {
  /* Negative URA */
  if (0 > ura) {
    return INVALID_GPS_URA_INDEX;
  }

  for (u8 i = 0; i < URA_VALUE_TABLE_LEN; i++) {
    if (gps_ura_values[i] >= ura) {
      return i;
    }
  }

  /* No valid URA index found */
  return INVALID_GPS_URA_INDEX;
}

/** Calculate the GPS ephemeris curve fit interval.
 *
 * \param fit_interval_flag The curve fit interval flag. 0 is 4 hours, 1 is >4
 * hours.
 * \param iodc The IODC value.
 * \return the curve fit interval in seconds.
 */
u32 decode_fit_interval(u8 fit_interval_flag, u16 iodc) {
  u8 fit_interval = 4; /* This is in hours */

  if (fit_interval_flag) {
    fit_interval = 6;

    if ((iodc >= 240) && (iodc <= 247)) {
      fit_interval = 8;
    } else if (((iodc >= 248) && (iodc <= 255)) || (iodc == 496)) {
      fit_interval = 14;
    } else if (((iodc >= 497) && (iodc <= 503)) ||
               ((iodc >= 1021) && (iodc <= 1023))) {
      fit_interval = 26;
    } else if ((iodc >= 504) && (iodc <= 510)) {
      fit_interval = 50;
    } else if ((iodc == 511) || ((iodc >= 752) && (iodc <= 756))) {
      fit_interval = 74;
    } else if (iodc == 757) {
      fit_interval = 98;
    }
  }

  return fit_interval * 60 * 60;
}

/** Decode ephemeris from L1 C/A GPS navigation message frames.
 *
 * \note This function does not check for parity errors. You should check the
 *       subframes for parity errors before calling this function.
 *
 * References:
 *   -# IS-GPS-200D, Section 20.3.2 and Figure 20-1
 *
 * \param frame_words Array containing words 3 through 10 of subframes
 *                    1, 2 and 3. Word is in the 30 LSBs of the u32.
 * \param e Pointer to an ephemeris struct to fill in.
 * \param tot_tow TOW for time of transmission
 */
void decode_ephemeris(const u32 frame_words[3][8],
                      ephemeris_t *e,
                      double tot_tow) {
  decode_ephemeris_with_wn_ref(frame_words, e, tot_tow, GPS_WEEK_REFERENCE);
}

/** Decode ephemeris from L1 C/A GPS navigation message frames.
 *
 * \note This function does not check for parity errors. You should check the
 *       subframes for parity errors before calling this function.
 *
 * References:
 *   -# IS-GPS-200D, Section 20.3.2 and Figure 20-1
 *
 * \param frame_words Array containing words 3 through 10 of subframes
 *                    1, 2 and 3. Word is in the 30 LSBs of the u32.
 * \param e Pointer to an ephemeris struct to fill in.
 * \param tot_tow TOW for time of transmission
 * \param wn_ref Reference week number that is from some point in the past
 */
void decode_ephemeris_with_wn_ref(const u32 frame_words[3][8],
                                  ephemeris_t *e,
                                  double tot_tow,
                                  u16 wn_ref) {
  assert(frame_words != NULL);
  assert(e != NULL);
  assert(IS_GPS(e->sid) || IS_QZSS(e->sid));
  ephemeris_kepler_t *k = &e->data.kepler;

  /* Subframe 1: WN, URA, SV health, T_GD, IODC, t_oc, a_f2, a_f1, a_f0 */

  /* GPS week number (mod 1024): Word 3, bits 1-10 */
  u16 wn_raw = frame_words[0][3 - 3] >> (30 - 10) & 0x3FF;

  /*
   * The ten MSBs of word three shall contain the ten LSBs of the
   * Week Number as defined in 3.3.4. These ten bits shall be a modulo
   * 1024 binary representation of the current GPS week number
   * at the start of the data set transmission interval. <<<<< IMPORTANT !!!
   */
  e->toe.wn = gps_adjust_week_cycle(wn_raw, wn_ref);

  /* t_oe: Word 10, bits 1-16 */
  e->toe.tow =
      (frame_words[1][10 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_TOE;

  bool toe_valid = gps_time_valid(&e->toe);
  if (toe_valid) {
    /* Match TOE week number with the time of transmission, fixes the case
     * near week roll-over where next week's ephemeris still has current week's
     * week number. */
    gps_time_t tot = {.wn = e->toe.wn, .tow = tot_tow};
    gps_time_match_weeks(&e->toe, &tot);
  } else {
    /* Invalid TOE, most likely TOW overflow.
     * Continue to decode the ephemeris, but mark it invalid at the end */
    log_warn_sid(e->sid,
                 "Latest ephemeris has faulty TOE: wn %d, tow %f."
                 " Invalidating ephemeris.",
                 e->toe.wn,
                 e->toe.tow);
  }

  k->toc.wn = e->toe.wn;

  /* URA: Word 3, bits 13-16 */
  /* Value of 15 is unhealthy */
  u8 ura_index = frame_words[0][3 - 3] >> (30 - 16) & 0xF;
  e->ura = decode_ura_index(ura_index);
  log_debug_sid(e->sid, "URA = index %d, value %.1f", ura_index, e->ura);

  /* NAV data and signal health bits: Word 3, bits 17-22 */
  e->health_bits = frame_words[0][3 - 3] >> (30 - 22) & 0x3F;
  log_debug_sid(e->sid, "Health bits = 0x%02" PRIx8, e->health_bits);

  /* t_gd: Word 7, bits 17-24 */
  k->tgd.gps_s[0] = (float)((s8)(frame_words[0][7 - 3] >> (30 - 24) & 0xFF) *
                            GPS_LNAV_EPH_SF_TGD);
  /* L1-L5 TGD has to be filled up with C-NAV as combination of L1-L2 TGD and
   * ISC_L5 */
  k->tgd.gps_s[1] = 0.0;

  /* iodc: Word 3, bits 23-24 and word 8, bits 1-8 */
  k->iodc = ((frame_words[0][3 - 3] >> (30 - 24) & 0x3) << 8) |
            (frame_words[0][8 - 3] >> (30 - 8) & 0xFF);

  /* t_oc: Word 8, bits 8-24 */
  k->toc.tow =
      (frame_words[0][8 - 3] >> (30 - 24) & 0xFFFF) * GPS_LNAV_EPH_SF_TOC;

  /* a_f2: Word 9, bits 1-8 */
  k->af2 = (s8)(frame_words[0][9 - 3] >> (30 - 8) & 0xFF) * GPS_LNAV_EPH_SF_AF2;

  /* a_f1: Word 9, bits 9-24 */
  k->af1 =
      (s16)(frame_words[0][9 - 3] >> (30 - 24) & 0xFFFF) * GPS_LNAV_EPH_SF_AF1;

  /* a_f0: Word 10, bits 1-22 */
  k->af0 = sign_extend22(frame_words[0][10 - 3] >> (30 - 22) & 0x3FFFFF) *
           GPS_LNAV_EPH_SF_AF0;

  /* Subframe 2: IODE, crs, dn, m0, cuc, ecc, cus, sqrta, toe, fit_interval */

  /* iode: Word 3, bits 1-8 */
  u8 iode_sf2 = frame_words[1][3 - 3] >> (30 - 8) & 0xFF;

  /* crs: Word 3, bits 9-24 */
  k->crs =
      (s16)(frame_words[1][3 - 3] >> (30 - 24) & 0xFFFF) * GPS_LNAV_EPH_SF_CRS;

  /* dn: Word 4, bits 1-16 */
  k->dn = (s16)(frame_words[1][4 - 3] >> (30 - 16) & 0xFFFF) *
          (GPS_LNAV_EPH_SF_DN * GPS_PI);

  /* m0: Word 4, bits 17-24 and word 5, bits 1-24 */
  k->m0 = (s32)(((frame_words[1][4 - 3] >> (30 - 24) & 0xFF) << 24) |
                (frame_words[1][5 - 3] >> (30 - 24) & 0xFFFFFF)) *
          (GPS_LNAV_EPH_SF_M0 * GPS_PI);

  /* cuc: Word 6, bits 1-16 */
  k->cuc =
      (s16)(frame_words[1][6 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_CUC;

  /* ecc: Word 6, bits 17-24 and word 7, bits 1-24 */
  k->ecc = (u32)(((frame_words[1][6 - 3] >> (30 - 24) & 0xFF) << 24) |
                 (frame_words[1][7 - 3] >> (30 - 24) & 0xFFFFFF)) *
           GPS_LNAV_EPH_SF_ECC;

  /* cus: Word 8, bits 1-16 */
  k->cus =
      (s16)(frame_words[1][8 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_CUS;

  /* sqrta: Word 8, bits 17-24 and word 9, bits 1-24 */
  k->sqrta = (u32)(((frame_words[1][8 - 3] >> (30 - 24) & 0xFF) << 24) |
                   (frame_words[1][9 - 3] >> (30 - 24) & 0xFFFFFF)) *
             GPS_LNAV_EPH_SF_SQRTA;

  /* fit_interval_flag: Word 10, bit 17 */
  u8 fit_interval_flag = frame_words[1][10 - 3] >> (30 - 17) & 0x1;
  e->fit_interval = decode_fit_interval(fit_interval_flag, k->iodc);
  log_debug_sid(e->sid, "Fit interval = %" PRIu32, e->fit_interval);

  /* Subframe 3: cic, omega0, cis, inc, crc, w, omegadot, IODE, inc_dot */

  /* cic: Word 3, bits 1-16 */
  k->cic =
      (s16)(frame_words[2][3 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_CIC;

  /* omega0: Word 3, bits 17-24 and word 4, bits 1-24 */
  k->omega0 = (s32)(((frame_words[2][3 - 3] >> (30 - 24) & 0xFF) << 24) |
                    (frame_words[2][4 - 3] >> (30 - 24) & 0xFFFFFF)) *
              (GPS_LNAV_EPH_SF_OMEGA0 * GPS_PI);

  /* cis: Word 5, bits 1-16 */
  k->cis =
      (s16)(frame_words[2][5 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_CIS;

  /* inc (i0): Word 5, bits 17-24 and word 6, bits 1-24 */
  k->inc = (s32)(((frame_words[2][5 - 3] >> (30 - 24) & 0xFF) << 24) |
                 (frame_words[2][6 - 3] >> (30 - 24) & 0xFFFFFF)) *
           (GPS_LNAV_EPH_SF_I0 * GPS_PI);

  /* crc: Word 7, bits 1-16 */
  k->crc =
      (s16)(frame_words[2][7 - 3] >> (30 - 16) & 0xFFFF) * GPS_LNAV_EPH_SF_CRC;

  /* w (omega): Word 7, bits 17-24 and word 8, bits 1-24 */
  k->w = (s32)(((frame_words[2][7 - 3] >> (30 - 24) & 0xFF) << 24) |
               (frame_words[2][8 - 3] >> (30 - 24) & 0xFFFFFF)) *
         (GPS_LNAV_EPH_SF_W * GPS_PI);

  /* Omega_dot: Word 9, bits 1-24 */
  k->omegadot = sign_extend24(frame_words[2][9 - 3] >> (30 - 24) & 0xFFFFFF) *
                (GPS_LNAV_EPH_SF_OMEGADOT * GPS_PI);

  /* iode: Word 10, bits 1-8 */
  k->iode = frame_words[2][10 - 3] >> (30 - 8) & 0xFF;

  /* inc_dot (IDOT): Word 10, bits 9-22 */
  k->inc_dot = sign_extend14(frame_words[2][10 - 3] >> (30 - 22) & 0x3FFF) *
               (GPS_LNAV_EPH_SF_IDOT * GPS_PI);

  /* Both IODEs and IODC (8 LSBs) must match */
  log_debug_sid(e->sid,
                "Check ephemeris. IODC = 0x%03" PRIX16 " IODE = 0x%02" PRIX8
                " and 0x%02" PRIX16 ".",
                k->iodc,
                iode_sf2,
                k->iode);

  bool iode_valid = (iode_sf2 == k->iode) && (k->iode == (k->iodc & 0xFF));
  if (!iode_valid) {
    log_warn_sid(e->sid,
                 "Latest ephemeris has IODC/IODE mismatch."
                 " Invalidating ephemeris.");
  }

  e->valid = iode_valid && toe_valid;
  e->source = EPH_SOURCE_GPS_LNAV;
}

/** Convert a BDS URA index into a URA value.
 *
 * \param index URA index.
 * \return the URA in meters.
 */
float decode_bds_ura_index(u8 index) {
  if (index >= sizeof(bds_ura_table) / sizeof(float)) {
    return INVALID_URA_VALUE;
  }
  return bds_ura_table[index];
}

/**
 * Decodes Beidou D1 ephemeris.
 * \param words subframes (FraID) 1,2,3.
 * \param sid signal ID.
 * \param ephe decoded ephemeris data.
 */
void decode_bds_d1_ephemeris(const u32 words[3][10],
                             gnss_signal_t sid,
                             ephemeris_t *ephe) {
  ephemeris_kepler_t *k = &ephe->data.kepler;

  /* subframe (FraID) 1 decoding */

  const u32 *sf1_word = &words[0][0];
  u8 sath1 = (((sf1_word[1]) >> 17) & 0x1);
  u8 urai = (((sf1_word[1]) >> 8) & 0xf);
  u16 weekno = (((sf1_word[2]) >> 17) & 0x1fff);
  u32 toc = (((sf1_word[2]) >> 8) & 0x1ff) << 8;
  toc |= (((sf1_word[3]) >> 22) & 0xff);
  u16 tgd1 = (((sf1_word[3]) >> 12) & 0x3ff);
  u16 tgd2 = (((sf1_word[3]) >> 8) & 0xf) << 6;
  tgd2 |= (((sf1_word[4]) >> 24) & 0x3f);
  u32 a[3];
  a[2] = (((sf1_word[7]) >> 15) & 0x7ff);
  a[0] = (((sf1_word[7]) >> 8) & 0x7f) << 17;
  a[0] |= (((sf1_word[8]) >> 13) & 0x1ffff);
  a[1] = (((sf1_word[8]) >> 8) & 0x1f) << 17;
  a[1] |= (((sf1_word[9]) >> 13) & 0x1ffff);

  /* Ephemeris params */
  ephe->sid = sid;
  ephe->health_bits = sath1;
  ephe->ura = decode_bds_ura_index(urai);
  ephe->toe.wn = BDS_WEEK_TO_GPS_WEEK + weekno;
  /* Keplerian params */
  k->tgd.bds_s[0] = BITS_SIGN_EXTEND_32(10, tgd1) * 1e-10f;
  k->tgd.bds_s[1] = BITS_SIGN_EXTEND_32(10, tgd2) * 1e-10f;
  k->toc.wn = ephe->toe.wn;
  k->toc.tow = (double)toc * C_2P3;
  k->af0 = BITS_SIGN_EXTEND_32(24, a[0]) * C_1_2P33;
  k->af1 = BITS_SIGN_EXTEND_32(22, a[1]) * C_1_2P50;
  k->af2 = BITS_SIGN_EXTEND_32(11, a[2]) * C_1_2P66;
  /* RTCM recommendation, BDS IODC = mod(toc / 720, 240)
   * Note scale factor effect, (toc * 8) / 720 -> (toc / 90) */
  k->iodc = (toc / 90) % BDS2_IODC_MAX;

  /* subframe (FraID) 2 decoding */

  const u32 *sf2_word = &words[1][0];
  u32 deltan = (((sf2_word[1]) >> 8) & 0x3ff) << 6;
  deltan |= (((sf2_word[2]) >> 24) & 0x3f);
  u32 cuc = (((sf2_word[2]) >> 8) & 0xffff) << 2;
  cuc |= (((sf2_word[3]) >> 28) & 0x3);
  u32 m0 = (((sf2_word[3]) >> 8) & 0xfffff) << 12;
  m0 |= (((sf2_word[4]) >> 18) & 0xfff);
  u32 ecc = (((sf2_word[4]) >> 8) & 0x3ff) << 22;
  ecc |= (((sf2_word[5]) >> 8) & 0x3fffff);
  u32 cus = (((sf2_word[6]) >> 12) & 0x3ffff);
  u32 crc = (((sf2_word[6]) >> 8) & 0xf) << 14;
  crc |= (((sf2_word[7]) >> 16) & 0x3fff);
  u32 crs = (((sf2_word[7]) >> 8) & 0xff) << 10;
  crs |= (((sf2_word[8]) >> 20) & 0x3ff);
  u32 sqrta = (((sf2_word[8]) >> 8) & 0xfff) << 20;
  sqrta |= (((sf2_word[9]) >> 10) & 0xfffff);
  u32 toe_msb = (((sf2_word[9]) >> 8) & 0x3);

  /* Beidou specific data */
  u32 split_toe = toe_msb << 15U;
  /* Keplerian params */
  k->dn = BITS_SIGN_EXTEND_32(16, deltan) * C_1_2P43 * GPS_PI;
  k->cuc = BITS_SIGN_EXTEND_32(18, cuc) * C_1_2P31;
  k->m0 = BITS_SIGN_EXTEND_32(32, m0) * C_1_2P31 * GPS_PI;
  k->ecc = ecc * C_1_2P33;
  k->cus = BITS_SIGN_EXTEND_32(18, cus) * C_1_2P31;
  k->crc = BITS_SIGN_EXTEND_32(18, crc) * C_1_2P6;
  k->crs = BITS_SIGN_EXTEND_32(18, crs) * C_1_2P6;
  k->sqrta = sqrta * C_1_2P19;

  /* subframe (FraID) 3 decoding */

  const u32 *sf3_word = &words[2][0];
  u32 toe_lsb = (((sf3_word[1]) >> 8) & 0x3ff) << 5;
  toe_lsb |= (((sf3_word[2]) >> 25) & 0x1f);
  u32 i0 = (((sf3_word[2]) >> 8) & 0x1ffff) << 15;
  i0 |= (((sf3_word[3]) >> 15) & 0x7fff);
  u32 cic = (((sf3_word[3]) >> 8) & 0x7f) << 11;
  cic |= (((sf3_word[4]) >> 19) & 0x7ff);
  u32 omegadot = (((sf3_word[4]) >> 8) & 0x7ff) << 13;
  omegadot |= (((sf3_word[5]) >> 17) & 0x1fff);
  u32 cis = (((sf3_word[5]) >> 8) & 0x1ff) << 9;
  cis |= (((sf3_word[6]) >> 21) & 0x1ff);
  u32 idot = (((sf3_word[6]) >> 8) & 0x1fff) << 1;
  idot |= (((sf3_word[7]) >> 29) & 0x1);
  u32 omegazero = (((sf3_word[7]) >> 8) & 0x1fffff) << 11;
  omegazero |= (((sf3_word[8]) >> 19) & 0x7ff);
  u32 omega = (((sf3_word[8]) >> 8) & 0x7ff) << 21;
  omega |= (((sf3_word[9]) >> 9) & 0x1fffff);

  /* Beidou specific data */
  split_toe |= toe_lsb;
  /* Ephemeris params */
  ephe->toe.tow = split_toe * C_2P3;
  /* RTCM recommendation, BDS IODE = mod(toe / 720, 240)
   * Note scale factor effect, (toe * 8) / 720 -> (toe / 90) */
  k->iode = (split_toe / 90) % BDS2_IODE_MAX;

  /* Keplerian params */
  k->inc = BITS_SIGN_EXTEND_32(32, i0) * C_1_2P31 * GPS_PI;
  k->cic = BITS_SIGN_EXTEND_32(18, cic) * C_1_2P31;
  k->omegadot = BITS_SIGN_EXTEND_32(24, omegadot) * C_1_2P43 * GPS_PI;
  k->cis = BITS_SIGN_EXTEND_32(18, cis) * C_1_2P31;
  k->inc_dot = BITS_SIGN_EXTEND_32(14, idot) * C_1_2P43 * GPS_PI;
  k->omega0 = BITS_SIGN_EXTEND_32(32, omegazero) * C_1_2P31 * GPS_PI;
  k->w = BITS_SIGN_EXTEND_32(32, omega) * C_1_2P31 * GPS_PI;

  ephe->source = EPH_SOURCE_BDS_D1_D2_NAV;
}

/** Convert a GAL SISA index into a URA value.
 *
 * \param sisa SISA index.
 * \return the URA in meters.
 */
float decode_sisa_index(u8 sisa) {
  float ura = INVALID_URA_VALUE;
  if (sisa < 50) {
    ura = sisa * 0.01f;
  } else if (sisa < 75) {
    ura = 0.5f + (sisa - 50) * 0.02f;
  } else if (sisa < 100) {
    ura = 1.0f + (sisa - 75) * 0.04f;
  } else if (sisa < 126) {
    ura = 2.0f + (sisa - 100) * 0.16f;
  } else if (INVALID_GAL_SISA_INDEX != sisa) {
    /* Note: SISA Index 126...254 are considered as Spare. */
    ura = 6.0f;
  }
  if (URA_VALID(ura)) {
    ura = rintf(ura / 0.01f) * 0.01f;
  }
  return ura;
}

/**
 * Decodes GAL I/NAV ephemeris.
 * \param page GAL pages 1-5. Page 5 is needed to extract
 *             Galileo system time (GST) and make corrections
 *             to TOE and TOC if needed.
 * \param eph the decoded ephemeris is placed here.
 * \return true if decoded successfully, false otherwise
 */
bool decode_gal_ephemeris_safe(const u8 page[5][GAL_INAV_CONTENT_BYTE],
                               ephemeris_t *eph) {
  ephemeris_kepler_t *kep = &eph->data.kepler;
  kep->iode = getbitu(page[0], 6, 10);
  kep->iodc = kep->iode;
  eph->fit_interval = GAL_FIT_INTERVAL_SECONDS;
  /* word type 1 */
  u32 toe = getbitu(page[0], 16, 14);
  u32 m0 = getbitu(page[0], 30, 32);
  u32 ecc = getbitu(page[0], 62, 32);
  u32 sqrta = getbitu(page[0], 94, 32);
  eph->toe.tow = toe * 60.0;
  kep->m0 = BITS_SIGN_EXTEND_32(32, m0) * C_1_2P31 * GPS_PI;
  kep->ecc = ecc * C_1_2P33;
  kep->sqrta = sqrta * C_1_2P19;
  /* word type 2 */
  u32 omega0 = getbitu(page[1], 16, 32);
  u32 i0 = getbitu(page[1], 48, 32);
  u32 omega = getbitu(page[1], 80, 32);
  u32 idot = getbitu(page[1], 112, 14);
  kep->omega0 = BITS_SIGN_EXTEND_32(32, omega0) * C_1_2P31 * GPS_PI;
  kep->inc = BITS_SIGN_EXTEND_32(32, i0) * C_1_2P31 * GPS_PI;
  kep->w = BITS_SIGN_EXTEND_32(32, omega) * C_1_2P31 * GPS_PI;
  kep->inc_dot = BITS_SIGN_EXTEND_32(14, idot) * C_1_2P43 * GPS_PI;
  /* word type 3 */
  u32 omegadot = getbitu(page[2], 16, 24);
  u32 deltan = getbitu(page[2], 40, 16);
  u32 cuc = getbitu(page[2], 56, 16);
  u32 cus = getbitu(page[2], 72, 16);
  u32 crc = getbitu(page[2], 88, 16);
  u32 crs = getbitu(page[2], 104, 16);
  u32 sisa = getbitu(page[2], 120, 8);
  kep->omegadot = BITS_SIGN_EXTEND_32(24, omegadot) * C_1_2P43 * GPS_PI;
  kep->dn = BITS_SIGN_EXTEND_32(16, deltan) * C_1_2P43 * GPS_PI;
  kep->cuc = BITS_SIGN_EXTEND_32(16, cuc) * C_1_2P29;
  kep->cus = BITS_SIGN_EXTEND_32(16, cus) * C_1_2P29;
  kep->crc = BITS_SIGN_EXTEND_32(16, crc) * C_1_2P5;
  kep->crs = BITS_SIGN_EXTEND_32(16, crs) * C_1_2P5;
  eph->ura = decode_sisa_index((u8)sisa);
  /* word type 4 */
  u32 sat = getbitu(page[3], 16, 6);
  u32 cic = getbitu(page[3], 22, 16);
  u32 cis = getbitu(page[3], 38, 16);
  u32 toc = getbitu(page[3], 54, 14);
  u32 af0 = getbitu(page[3], 68, 31);
  u32 af1 = getbitu(page[3], 99, 21);
  u32 af2 = getbitu(page[3], 120, 6);
  eph->sid.sat = sat;
  kep->cic = BITS_SIGN_EXTEND_32(16, cic) * C_1_2P29;
  kep->cis = BITS_SIGN_EXTEND_32(16, cis) * C_1_2P29;
  kep->toc.tow = toc * 60.0;
  kep->af0 = BITS_SIGN_EXTEND_32(31, af0) * C_1_2P34;
  kep->af1 = BITS_SIGN_EXTEND_32(21, af1) * C_1_2P46;
  kep->af2 = BITS_SIGN_EXTEND_32(6, af2) * C_1_2P59;
  /* word type 5 */
  u16 bgd_e5a_e1 = getbitu(page[4], 47, 10); /* E5a/E1 BGD */
  u16 bgd_e5b_e1 = getbitu(page[4], 57, 10); /* E5b/E1 BGD */

  kep->tgd.gal_s[0] =
      (float)BITS_SIGN_EXTEND_32(10, bgd_e5a_e1) * (float)C_1_2P32;
  kep->tgd.gal_s[1] =
      (float)BITS_SIGN_EXTEND_32(10, bgd_e5b_e1) * (float)C_1_2P32;

  u16 e5b_hs = getbitu(page[4], 67, 2); /* E5b Health Status */
  u16 e1b_hs = getbitu(page[4], 69, 2); /* E1b Health Status */

  eph->valid = (bool)((e5b_hs == GAL_HS_SIGNAL_OK ||
                       e5b_hs == GAL_HS_SIGNAL_WILL_BE_OUT_OF_SERVICE) &&
                      (e1b_hs == GAL_HS_SIGNAL_OK ||
                       e1b_hs == GAL_HS_SIGNAL_WILL_BE_OUT_OF_SERVICE));

  gps_time_t t;
  t.wn = (s16)getbitu(page[4], 73, 12) + GAL_WEEK_TO_GPS_WEEK;
  t.tow = (double)getbitu(page[4], 85, 20);

  /* Match TOE week number with the time of transmission, fixes the case
   * near week roll-over where time of ephemeris is across the week boundary */
  if (!gps_time_match_weeks_safe(&eph->toe, &t)) {
    return false;
  }
  if (!gps_time_match_weeks_safe(&kep->toc, &t)) {
    return false;
  }

  eph->source = EPH_SOURCE_GAL_INAV;

  return true;
}

/**
 * Decodes GAL I/NAV ephemeris.
 * This function will assert if the ephemeris could not be decoded successfully
 * \param page GAL pages 1-5. Page 5 is needed to extract
 *             Galileo system time (GST) and make corrections
 *             to TOE and TOC if needed.
 * \param eph the decoded ephemeris is placed here.
 */
void decode_gal_ephemeris(const u8 page[5][GAL_INAV_CONTENT_BYTE],
                          ephemeris_t *eph) {
  bool result = decode_gal_ephemeris_safe(page, eph);
  (void)result;
  assert(result);
}

/**
 * Decodes GLO ephemeris.
 * \param strings GLO navigation strings 1-5
 * \param sid identifier of the satellite
 * \param utc_params pointer to UTC parameters (NULL for factory values)
 * \param eph the decoded ephemeris is placed here.
 */
void decode_glo_ephemeris(const glo_string_t strings[5],
                          const gnss_signal_t sid,
                          const utc_params_t *utc_params,
                          ephemeris_t *eph) {
  eph->toe = GPS_TIME_UNKNOWN;
  eph->sid = sid;
  eph->valid = 0;

  glo_time_t tk;
  glo_time_t toe;
  u8 age_of_data_days;
  float tau_gps_s;

  bool ret = decode_glo_string_1(&strings[0], eph, &tk);
  ret &= decode_glo_string_2(&strings[1], eph, &toe);
  ret &= decode_glo_string_3(&strings[2], eph, &toe);
  ret &= decode_glo_string_4(&strings[3], eph, &tk, &toe, &age_of_data_days);
  ret &= decode_glo_string_5(&strings[4], eph, &tk, &toe, &tau_gps_s);

  if (ret) {
    eph->toe = glo2gps(&toe, utc_params);
    eph->valid = 1;
  }

  eph->source = EPH_SOURCE_GLO_FDMA;
}

static bool ephemeris_xyz_equal(const ephemeris_xyz_t *a,
                                const ephemeris_xyz_t *b) {
  return fabs(a->pos[0] - b->pos[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->pos[1] - b->pos[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->pos[2] - b->pos[2]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[0] - b->vel[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[1] - b->vel[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[2] - b->vel[2]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[0] - b->acc[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[1] - b->acc[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[2] - b->acc[2]) < FLOAT_EQUALITY_EPS &&
         fabs(a->a_gf0 - b->a_gf0) < FLOAT_EQUALITY_EPS &&
         fabs(a->a_gf1 - b->a_gf1) < FLOAT_EQUALITY_EPS;
}

static bool ephemeris_kepler_equal(const constellation_t constel,
                                   const ephemeris_kepler_t *a,
                                   const ephemeris_kepler_t *b) {
  switch (constel) {
    case CONSTELLATION_GPS:
      if (fabsf(a->tgd.gps_s[0] - b->tgd.gps_s[0]) > FLOAT_EQUALITY_EPS ||
          fabsf(a->tgd.gps_s[1] - b->tgd.gps_s[1]) > FLOAT_EQUALITY_EPS) {
        return false;
      }
      break;
    case CONSTELLATION_QZS:
      if (fabsf(a->tgd.qzss_s[0] - b->tgd.qzss_s[0]) > FLOAT_EQUALITY_EPS ||
          fabsf(a->tgd.qzss_s[1] - b->tgd.qzss_s[1]) > FLOAT_EQUALITY_EPS) {
        return false;
      }
      break;
    case CONSTELLATION_GAL:
      if (fabsf(a->tgd.gal_s[0] - b->tgd.gal_s[0]) > FLOAT_EQUALITY_EPS ||
          fabsf(a->tgd.gal_s[1] - b->tgd.gal_s[1]) > FLOAT_EQUALITY_EPS) {
        return false;
      }
      break;
    case CONSTELLATION_BDS:
      if (fabsf(a->tgd.bds_s[0] - b->tgd.bds_s[0]) > FLOAT_EQUALITY_EPS ||
          fabsf(a->tgd.bds_s[1] - b->tgd.bds_s[1]) > FLOAT_EQUALITY_EPS) {
        return false;
      }
      break;
    case CONSTELLATION_SBAS:
    case CONSTELLATION_GLO:
    case CONSTELLATION_COUNT:
    case CONSTELLATION_INVALID:
    default:
      log_error("Invalid constellation given to ephemeris_kepler_equal\n");
      assert(false);
  }
  return fabs(a->crc - b->crc) < FLOAT_EQUALITY_EPS &&
         fabs(a->crs - b->crs) < FLOAT_EQUALITY_EPS &&
         fabs(a->cuc - b->cuc) < FLOAT_EQUALITY_EPS &&
         fabs(a->cus - b->cus) < FLOAT_EQUALITY_EPS &&
         fabs(a->cic - b->cic) < FLOAT_EQUALITY_EPS &&
         fabs(a->cis - b->cis) < FLOAT_EQUALITY_EPS &&
         fabs(a->dn - b->dn) < FLOAT_EQUALITY_EPS &&
         fabs(a->m0 - b->m0) < FLOAT_EQUALITY_EPS &&
         fabs(a->ecc - b->ecc) < FLOAT_EQUALITY_EPS &&
         fabs(a->sqrta - b->sqrta) < FLOAT_EQUALITY_EPS &&
         fabs(a->omega0 - b->omega0) < FLOAT_EQUALITY_EPS &&
         fabs(a->omegadot - b->omegadot) < FLOAT_EQUALITY_EPS &&
         fabs(a->w - b->w) < FLOAT_EQUALITY_EPS &&
         fabs(a->inc - b->inc) < FLOAT_EQUALITY_EPS &&
         fabs(a->inc_dot - b->inc_dot) < FLOAT_EQUALITY_EPS &&
         fabs(a->af0 - b->af0) < FLOAT_EQUALITY_EPS &&
         fabs(a->af1 - b->af1) < FLOAT_EQUALITY_EPS &&
         fabs(a->af2 - b->af2) < FLOAT_EQUALITY_EPS &&
         fabs(a->toc.tow - b->toc.tow) < FLOAT_EQUALITY_EPS &&
         a->toc.wn == b->toc.wn && a->iodc == b->iodc && a->iode == b->iode;
}

static bool ephemeris_glo_equal(const ephemeris_glo_t *a,
                                const ephemeris_glo_t *b) {
  return fabs(a->gamma - b->gamma) < FLOAT_EQUALITY_EPS &&
         fabs(a->tau - b->tau) < FLOAT_EQUALITY_EPS &&
         fabs(a->d_tau - b->d_tau) < FLOAT_EQUALITY_EPS &&
         fabs(a->pos[0] - b->pos[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->pos[1] - b->pos[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->pos[2] - b->pos[2]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[0] - b->vel[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[1] - b->vel[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->vel[2] - b->vel[2]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[0] - b->acc[0]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[1] - b->acc[1]) < FLOAT_EQUALITY_EPS &&
         fabs(a->acc[2] - b->acc[2]) < FLOAT_EQUALITY_EPS && a->fcn == b->fcn &&
         a->iod == b->iod;
}

/** Are the two ephemerides the same?
 *
 * \param a First ephemeris
 * \param b Second ephemeris
 * \return true if they are equal
 */
bool ephemeris_equal(const ephemeris_t *a, const ephemeris_t *b) {
  /* fit_interval and ura are ignored as some receivers (e.g. Trimble) do
     not always agree on the value of those fields. See SR-142 */
  if (!sid_is_equal(a->sid, b->sid) || (a->valid != b->valid) ||
      (a->health_bits != b->health_bits) || (a->toe.wn != b->toe.wn) ||
      (!double_equal(a->toe.tow, b->toe.tow))) {
    return false;
  }

  switch (sid_to_constellation(a->sid)) {
    case CONSTELLATION_GPS:
    case CONSTELLATION_QZS:
    case CONSTELLATION_BDS:
    case CONSTELLATION_GAL:
      return ephemeris_kepler_equal(
          sid_to_constellation(a->sid), &a->data.kepler, &b->data.kepler);
    case CONSTELLATION_SBAS:
      return ephemeris_xyz_equal(&a->data.xyz, &b->data.xyz);
    case CONSTELLATION_GLO:
      return ephemeris_glo_equal(&a->data.glo, &b->data.glo);
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      assert(!"Unsupported constellation");
      return false;
  }
}

/** Check if this this ephemeris is healthy
 *
 * \param ephe pointer to ephemeris to check
 * \param code signal code, ephe->sid can't be used as for example L2CM uses
 *             L1CA ephes
 * \return true if the ephemeris is healthy
 *         false otherwise
 */
bool ephemeris_healthy(const ephemeris_t *ephe, const code_t code) {
  /* Presume healthy */
  bool ret = true;

  if (!ephe->valid) {
    /* If we don't yet have an ephemeris, assume satellite is healthy */
    /* Otherwise we will stop tracking the sat and never find out */
    return ret;
  }

  switch (code_to_constellation(code)) {
    case CONSTELLATION_GPS:
      if (encode_ura(ephe->ura) > MAX_ALLOWED_GPS_URA_IDX) {
        ret = false;
        /* Satellite is not healthy, no reason to check further */
        break;
      }

      ret = check_6bit_health_word(ephe->health_bits, code);

      break;

    case CONSTELLATION_GLO:
    case CONSTELLATION_BDS:
    case CONSTELLATION_GAL:
      if (!URA_VALID(ephe->ura)) {
        ret = false;
        /* Satellite is not healthy, no reason to check further */
        break;
      }

      ret = (0 == ephe->health_bits);

      break;

    case CONSTELLATION_SBAS:
    case CONSTELLATION_QZS:
      ret = (0 == ephe->health_bits);
      break;

    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      assert(!"Unsupported constellation");
      ret = false;
      break;
  }

  return ret;
}

/* Compute a 24 bit CRC of the BDS 2 ephemeris following the CNES algorithm
 * detailed in https://doi.org/10.1007/s10291-017-0678-6 and
 * https://doi.org/10.1109/ACCESS.2019.2938252
 *
 * Based on the BNC implementation here:
 * https://github.com/swift-nav/PPP_Wizard14/blob/
 * b05025517fa3f5ee4334171b97ab7475db319215/RTRover/rtrover_broadcast.cpp#L391
 *
 * \param eph a BDS 2 ephemeris
 * \return a 24 bit CRC value
 */
u32 get_bds2_iod_crc(const ephemeris_t *eph) {
  assert(sid_to_constellation(eph->sid) == CONSTELLATION_BDS);

  unsigned char buffer[80] = {0};
  s32 numbits = 0;

  setbits(buffer,
          numbits,
          14,
          (s32)(eph->data.kepler.inc_dot / M_PI * (double)(1 << 30) *
                (double)(1 << 13)));
  numbits += 14;
  setbits(buffer,
          numbits,
          11,
          (s32)(eph->data.kepler.af2 * (double)(1 << 30) * (double)(1 << 30) *
                (double)(1 << 6)));
  numbits += 11;
  setbits(buffer,
          numbits,
          22,
          (s32)(eph->data.kepler.af1 * (double)(1 << 30) * (double)(1 << 20)));
  numbits += 22;
  setbits(buffer,
          numbits,
          24,
          (s32)(eph->data.kepler.af0 * (double)(1 << 30) * (double)(1 << 3)));
  numbits += 24;
  setbits(buffer, numbits, 18, (s32)(eph->data.kepler.crs * (double)(1 << 6)));
  numbits += 18;
  setbits(buffer,
          numbits,
          16,
          (s32)(eph->data.kepler.dn / M_PI * (double)(1 << 30) *
                (double)(1 << 13)));
  numbits += 16;
  setbits(
      buffer,
      numbits,
      32,
      (s32)(eph->data.kepler.m0 / M_PI * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 32;
  setbits(buffer,
          numbits,
          18,
          (s32)(eph->data.kepler.cuc * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 18;
  setbitu(buffer,
          numbits,
          32,
          (u32)(eph->data.kepler.ecc * (double)(1 << 30) * (double)(1 << 3)));
  numbits += 32;
  setbits(buffer,
          numbits,
          18,
          (s32)(eph->data.kepler.cus * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 18;
  setbitu(
      buffer, numbits, 32, (u32)(eph->data.kepler.sqrta * (double)(1 << 19)));
  numbits += 32;
  setbits(buffer,
          numbits,
          18,
          (s32)(eph->data.kepler.cic * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 18;
  setbits(buffer,
          numbits,
          32,
          (s32)(eph->data.kepler.omega0 / M_PI * (double)(1 << 30) *
                (double)(1 << 1)));
  numbits += 32;
  setbits(buffer,
          numbits,
          18,
          (s32)(eph->data.kepler.cis * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 18;
  setbits(buffer,
          numbits,
          32,
          (s32)(eph->data.kepler.inc / M_PI * (double)(1 << 30) *
                (double)(1 << 1)));
  numbits += 32;
  setbits(buffer, numbits, 18, (s32)(eph->data.kepler.crc * (double)(1 << 6)));
  numbits += 18;
  setbits(
      buffer,
      numbits,
      32,
      (s32)(eph->data.kepler.w / M_PI * (double)(1 << 30) * (double)(1 << 1)));
  numbits += 32;
  setbits(buffer,
          numbits,
          24,
          (s32)(eph->data.kepler.omegadot / M_PI * (double)(1 << 30) *
                (double)(1 << 13)));
  numbits += 24;
  setbits(buffer, numbits, 5, 0);
  numbits += 5;

  return crc24q(buffer, numbits / 8, 0);
}

/** Get the time group delay to be applied to the satellite clock correction
 * \param eph Ephemeris
 * \param sid Sid of the signal to correct
 * \param tgd Applied group delay correction
 * \return  0 on success,
 *         -1 if tgd is not valid
 */
s8 get_tgd_correction(const ephemeris_t *eph,
                      const gnss_signal_t *sid,
                      float *tgd) {
  double frequency, gamma;
  assert(sid_to_constellation(eph->sid) == sid_to_constellation(*sid));
  switch (sid_to_constellation(*sid)) {
    case CONSTELLATION_GPS:
      /* sat_clock_error = iono_free_clock_error - (f_1 / f)^2 * TGD. */
      frequency = sid_to_carr_freq(*sid);
      gamma = GPS_L1_HZ * GPS_L1_HZ / (frequency * frequency);
      if (CODE_GPS_L5I == sid->code || CODE_GPS_L5Q == sid->code ||
          CODE_GPS_L5X == sid->code) {
        *tgd = (float)(eph->data.kepler.tgd.gps_s[1] * gamma);
      } else {
        *tgd = (float)(eph->data.kepler.tgd.gps_s[0] * gamma);
      }
      return 0;
    case CONSTELLATION_BDS:
      // As BeiDou CNAV ephemeris is not yet available, use BeiDou B1i group
      // delay for B1C. It is not a perfect way and there may still be a meter
      // level error.
      if (CODE_BDS2_B1 == sid->code || CODE_BDS3_B1CI == sid->code) {
        *tgd = eph->data.kepler.tgd.bds_s[0];
      } else if (CODE_BDS3_B3I == sid->code || CODE_BDS3_B3Q == sid->code ||
                 CODE_BDS3_B3X == sid->code) {
        // BDS B3I signal is the reference broadcast clock so its TGD is 0 by
        // definition
        *tgd = 0.0f;
      } else {
        // Fallback to B2I TGD for all other signals (including B2a)
        *tgd = eph->data.kepler.tgd.bds_s[1];
      }
      return 0;
    case CONSTELLATION_GLO:
      /* As per GLO ICD v5.1 2008:
         d_tau = t_f2 - t_f1 -> t_f1 = t_f2 - d_tau.
         As clock_err is added to pseudorange,
         d_tau has to be applied with negative sign. */
      if (CODE_GLO_L1OF == sid->code || CODE_GLO_L1P == sid->code) {
        *tgd = 0.0;
        return 0;
      } else if (CODE_GLO_L2OF == sid->code || CODE_GLO_L2P == sid->code) {
        *tgd = (float)eph->data.glo.d_tau;
        return 0;
      } else {
        log_debug_sid(*sid, "TGD not applied for the signal");
      }
      return -1;
    case CONSTELLATION_QZS:
      /* As per QZSS ICD draft 1.5, all signals use the same unscaled Tgd and
       * inter-signal biases are applied separately - it was however decided
       * to take a Galileo-like approach with per-frequency-combination tgds */
      frequency = sid_to_carr_freq(*sid);
      gamma = QZS_L1_HZ * QZS_L1_HZ / (frequency * frequency);
      if (CODE_QZS_L5I == sid->code || CODE_QZS_L5Q == sid->code ||
          CODE_QZS_L5X == sid->code) {
        *tgd = (float)(eph->data.kepler.tgd.gps_s[1] * gamma);
      } else {
        *tgd = (float)(eph->data.kepler.tgd.gps_s[0] * gamma);
      }
      return 0;
    case CONSTELLATION_GAL:
      /* Galileo ICD chapter 5.1.5 */
      frequency = sid_to_carr_freq(*sid);
      gamma = (GAL_E1_HZ * GAL_E1_HZ) / (frequency * frequency);
      if (CODE_GAL_E5I == sid->code || CODE_GAL_E5Q == sid->code ||
          CODE_GAL_E5X == sid->code) {
        /* The first TGD correction is for the (E1,E5a) combination */
        *tgd = (float)(gamma * eph->data.kepler.tgd.gal_s[0]);
        return 0;
      }
      /* The clock corrections from INAV are for the (E1,E5b) combination, so
       * use the matching group delay correction for all the other signals */
      *tgd = (float)(gamma * eph->data.kepler.tgd.gal_s[1]);
      return 0;
    case CONSTELLATION_INVALID:
    case CONSTELLATION_SBAS:
    case CONSTELLATION_COUNT:
    default:
      assert(!"Unsupported constellation");
      return -1;
  }
}

/** \} */
