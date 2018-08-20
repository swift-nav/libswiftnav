/*
 * Copyright (c) 2010 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_COORD_SYSTEM_H
#define LIBSWIFTNAV_COORD_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \addtogroup coord_system
 * \{ */

/** \defgroup WGS84_params WGS84 Parameters
 * Parameters defining the WGS84 ellipsoid. The ellipsoid is defined in terms
 * of the semi-major axis and the inverse flattening. We also calculate some
 * derived parameters which are useful for the implementation of the coordinate
 * transform functions.
 * \{ */
/** Semi-major axis of the Earth, \f$ a \f$, in meters.
 * This is a defining parameter of the WGS84 ellipsoid. */
#define WGS84_A 6378137.0
/** Inverse flattening of the Earth, \f$ 1/f \f$.
 * This is a defining parameter of the WGS84 ellipsoid. */
#define WGS84_IF 298.257223563
/** The flattening of the Earth, \f$ f \f$. */
#define WGS84_F (1 / WGS84_IF)
/** Semi-minor axis of the Earth in meters, \f$ b = a(1-f) \f$. */
#define WGS84_B (WGS84_A * (1 - WGS84_F))
/** Eccentricity of the Earth, \f$ e \f$ where \f$ e^2 = 2f - f^2 \f$ */
#define WGS84_E (sqrt(2 * WGS84_F - WGS84_F * WGS84_F))
/** Geocentric Gravitational Contant (GM) in m^3 / s^2 */
#define WGS84_GM 3.986004418e14
/** Nominal Mean Angular Velocity of the Earth in rad / s */
#define WGS84_OMEGAE_DOT 7.292115e-5
/** The speed of light in m / s.
 * \note This is the exact value of the speed of light in vacuum (by the
 * definition of meters). */
#define WGS84_C 299792458.0
/** Universal Constant of Gravitation in m^2 / kg*s^2 */
#define WGS84_G 6.67428e-11
/** Mass of the Earth (M) in kg */
#define WGS84_M (WGS84_GM / WGS84_G)
/** Total Mean Mass of the Atmosphere (with water vapor) M_A in kg */
#define WGS84_M_A 5.1480e18
/** Dynamic Ellipticity (H) */
#define WGS84_H 3.273795e-3
/** Second Degree Zonal Harmonic */
#define WGS84_J02 1.082629821313e-3
/* \} */

/* \} */

void llhrad2deg(const double llh_rad[3], double llh_deg[3]);

void llhdeg2rad(const double llh_deg[3], double llh_rad[3]);

void wgsllh2ecef(const double llh[3], double ecef[3]);

void wgsecef2llh(const double ecef[3], double llh[3]);

void wgsecef2ned(const double ecef[3], const double ref_ecef[3], double ned[3]);
void wgsecef2ned_d(const double ecef[3],
                   const double ref_ecef[3],
                   double ned[3]);

void wgsned2ecef(const double ned[3], const double ref_ecef[3], double ecef[3]);
void wgsned2ecef_d(const double ned[3],
                   const double ref_ecef[3],
                   double ecef[3]);

void wgsecef2azel(const double ecef[3],
                  const double ref_ecef[3],
                  double* azimuth,
                  double* elevation);

void wgs_ecef2ned_matrix(const double llh[3], double M[3][3]);

void ecef2ned_matrix(const double ref_ecef[3], double M[3][3]);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_COORD_SYSTEM_H */
