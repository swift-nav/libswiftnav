/*
 * Copyright (C) 2013, 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_CONSTANTS_H
#define LIBSWIFTNAV_CONSTANTS_H

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \defgroup constants Constants
 * Useful constants.
 * \{ */

#ifdef __cplusplus
#ifndef M_PI
#define M_PI                                                                 \
  3.14159265358979323846264338 /**< This is not part of the C++ standard, so \
                                  we define it ourselves if needed. */
#endif                         /* M_PI */

#ifndef M_PI_2
#define M_PI_2                                                                 \
  1.5707963267948966192313216 /**< This is not part of the C++ standard, so we \
                                 define it ourselves if needed. */
#endif                        /* M_PI_2 */
#endif                        /* __cplusplus */

#define R2D (180.0 / M_PI) /**< Conversion factor from radians to degrees. */
#define D2R (M_PI / 180.0) /**< Conversion factor from degrees to radians. */
#define AS2R (D2R / 3600.0)

#define FLOAT_EQUALITY_EPS 1e-12

/* Constants computed in python and verified in matlab */
/** 2^3 */
#define C_2P3 8
/** 2^4 */
#define POW_TWO_4 16
#define C_2P4 16
/** 2^7 */
#define POW_TWO_7 128
#define C_2P7 128
/** 2^11 */
#define C_2P11 2048
/** 2^14 */
#define C_2P14 16384
/** 2^16 */
#define C_2P16 65536
/** 2^-5 */
#define C_1_2P5 0.03125
/** 2^-6 */
#define C_1_2P6 0.015625
/** 2^-11 */
#define C_1_2P11 0.00048828125
/** 2^-19 */
#define C_1_2P19 1.9073486328125e-06
/** 2^-20 */
#define C_1_2P20 9.5367431640625e-07
/** 2^-21 */
#define C_1_2P21 4.76837158203125e-07
/** 2^-23 */
#define C_1_2P23 1.1920928955078125e-07
/** 2^-24 */
#define C_1_2P24 5.960464477539063e-08
/** 2^-27 */
#define C_1_2P27 7.450580596923828e-09
/** 2^-29 */
#define C_1_2P29 1.862645149230957e-09
/** 2^-30 */
#define C_1_2P30 9.313225746154785e-10
/** 2^-31 */
#define C_1_2P31 4.656612873077393e-10
/** 2^-33 */
#define C_1_2P33 1.1641532182693481e-10
/** 2^-34 */
#define C_1_2P34 5.82076609134674e-011
/** 2^-35 */
#define C_1_2P35 2.9103830456733704e-11
/** 2^-38 */
#define C_1_2P38 3.637978807091713e-12
/** 2^-39 */
#define C_1_2P39 1.8189894035458565e-12
/** 2^-40 */
#define C_1_2P40 9.09494701772928e-013
/** 2^-43 */
#define C_1_2P43 1.1368683772161603e-13
/** 2^-46 */
#define C_1_2P46 1.4210854715202e-014
/** 2^-50 */
#define C_1_2P50 8.881784197001252e-16
/** 2^-51 */
#define C_1_2P51 4.440892098500626e-16
/** 2^-55 */
#define C_1_2P55 2.7755575615628914e-17
/** 2^-59 */
#define C_1_2P59 1.73472347597681e-018
/** 2^-66 */
#define C_1_2P66 1.35525271560688e-020
/** 2^-68 */
#define C_1_2P68 3.3881317890172014e-21

/**< Standard acceleration of gravity [m/s^2] */
#define STD_GRAVITY_ACCELERATION 9.80665

/** \defgroup gps_constants GPS
 * Constants related to the Global Positioning System.
 * See ICD-GPS-200C.
 * \{ */

/** The official GPS value of Pi.
 * \note This is the value used by the CS to curve fit ephemeris parameters and
 * should be used in all ephemeris calculations. */
#define GPS_PI 3.1415926535898

/** The GPS L1 center frequency in Hz. */
#define GPS_L1_HZ 1.57542e9

/** The GPS L2 center frequency in Hz. */
#define GPS_L2_HZ 1.22760e9

/** The GPS L5 center frequency in Hz. */
#define GPS_L5_HZ (115 * 10.23e6)

/** Earth's rotation rate as defined in the ICD in rad / s
 * \note This is actually not identical to the usual WGS84 definition. */
#define GPS_OMEGAE_DOT 7.2921151467e-5

/** Earth's Gravitational Constant as defined in the ICD in m^3 / s^2
 * \note This is actually not identical to the usual WGS84_GM definition. */
#define GPS_GM 3.986005e14

/** The official GPS value of the speed of light in m / s.
 * \note This is the exact value of the speed of light in vacuum (by the
 * definition of meters). */
#define GPS_C 299792458.0

/** The official GPS value of the relativistic clock correction coefficient F.
 */
#define GPS_F (-4.442807633e-10)

/** The speed of light in air at standard temperature and pressure.
 * \note This is GPS_C / mu where mu is 1.0002926 */
#define GPS_C_NO_VAC (GPS_C / 1.0002926)

/** The wavelength of L1 in a vacuum.
 * \note This is GPS_C / GPS_L1_HZ. */
#define GPS_L1_LAMBDA (GPS_C / GPS_L1_HZ)

/** The wavelength of L2 in a vacuum.
 * \note This is GPS_C / GPS_L2_HZ. */
#define GPS_L2_LAMBDA (GPS_C / GPS_L2_HZ)

/** The wavelength of L5 */
#define GPS_L5_LAMBDA (GPS_C / GPS_L5_HZ)

/** The wavelength of L1 in air at standard temperature and pressure.
 * \note This is GPS_C_NO_VAC / GPS_L1_HZ. */
#define GPS_L1_LAMBDA_NO_VAC (GPS_C_NO_VAC / GPS_L1_HZ)

/** GPS L1 C/A and L2 C code chipping rate in Hz. */
#define GPS_CA_CHIPPING_RATE 1.023e6

/** GPS L5 code chipping rate in Hz. */
#define GPS_L5_CHIPPING_RATE 10.23e6

/** GPS L1 C/A carrier freq / code chipping rate
 * \note this is GPS_L1_HZ / GPS_CA_CHIPPING_RATE */
#define GPS_L1CA_CARR_TO_CODE (GPS_L1_HZ / GPS_CA_CHIPPING_RATE)

/** GPS L2C carrier freq / code chipping rate
 * \note this is GPS_L2_HZ / GPS_CA_CHIPPING_RATE */
#define GPS_L2C_CARR_TO_CODE (GPS_L2_HZ / GPS_CA_CHIPPING_RATE)

/** GPS L5 carrier freq / code chipping rate
 * \note this is GPS_L5_HZ / GPS_L5_CHIPPING_RATE */
#define GPS_L5_CARR_TO_CODE (GPS_L5_HZ / GPS_L5_CHIPPING_RATE)

/** GPS CM code chipping rate in Hz. */
#define GPS_CM_CHIPPING_RATE .5115e6

/** GPS CL code chipping rate in Hz. */
#define GPS_CL_CHIPPING_RATE .5115e6

/** GPS L1 C/A chips number */
#define GPS_L1CA_CHIPS_NUM 1023

/** GPS L1C chips number */
#define GPS_L1C_CHIPS_NUM 10230

/** GPS L2 CM chips number */
#define GPS_L2CM_CHIPS_NUM (10230 + 10230)

/** GPS L2 CL chips number */
#define GPS_L2CL_CHIPS_NUM (767250 + 767250)

/** GPS L5 chips number */
#define GPS_L5_CHIPS_NUM 10230

/** GPS L1 C/A PRN period in ms */
#define GPS_L1CA_PRN_PERIOD_MS 1

/** GPS L1C PRN period in ms */
#define GPS_L1C_PRN_PERIOD_MS 10

/** GPS L2 CM PRN period in ms */
#define GPS_L2CM_PRN_PERIOD_MS 20

/** GPS L2 CL PRN period in ms */
#define GPS_L2CL_PRN_PERIOD_MS 1500

/** GPS L2 CL PRN start interval in ms */
#define GPS_L2CL_PRN_START_INTERVAL_MS 20

/** GPS L2 CL PRN chips per tracker start interval
    #GPS_L2CL_PRN_START_INTERVAL_MS */
#define GPS_L2CL_PRN_CHIPS_PER_INTERVAL \
  (GPS_L2CL_CHIPS_NUM * GPS_L2CL_PRN_START_INTERVAL_MS / GPS_L2CL_PRN_PERIOD_MS)

/** Number of start points in GPS L2 CL PRN */
#define GPS_L2CL_PRN_START_POINTS \
  (GPS_L2CL_PRN_PERIOD_MS / GPS_L2CL_PRN_START_INTERVAL_MS)

/** GPS L5 PRN period in ms */
#define GPS_L5_PRN_PERIOD_MS 1

/** GPS L1 C/A pseudosymbol length [ms] */
#define GPS_L1CA_PSYMBOL_LENGTH_MS 1

/** GPS L1 maximum Doppler value [Hz] */
#define GPS_L1_DOPPLER_MAX_HZ (+4200.f)

/** GPS L2 maximum Doppler value [Hz] */
#define GPS_L2_DOPPLER_MAX_HZ (GPS_L1_DOPPLER_MAX_HZ * GPS_L2_HZ / GPS_L1_HZ)

/** GPS L5 maximum Doppler value [Hz] */
#define GPS_L5_DOPPLER_MAX_HZ (GPS_L1_DOPPLER_MAX_HZ * GPS_L5_HZ / GPS_L1_HZ)

/** \} */

/** \defgroup glonass_constants GLONASS
 * Constants related to the GLONASS system.
 * See GLONASS ICD 5.1.
 * \{ */

/** GLO L1 C/A bit length [ms] */
#define GLO_L1CA_BIT_LENGTH_MS 20

/** GLO L1 and L2 PRN period in ms */
#define GLO_PRN_PERIOD_MS 1

/** The GLO L1 center frequency in Hz. */
#define GLO_L1_HZ 1.602e9

/** The GLO L2 center frequency in Hz. */
#define GLO_L2_HZ 1.246e9

/** The wavelength of L1 in a vacuum.
 * \note This is GPS_C / GLO_L1_HZ. */
#define GLO_L1_LAMBDA (GPS_C / GLO_L1_HZ)

/** The wavelength of L2 in a vacuum.
 * \note This is GPS_C / GLO_L2_HZ. */
#define GLO_L2_LAMBDA (GPS_C / GLO_L2_HZ)

/** GLO semi-major axis of Earth
 * \note there is define WGS84_A which is 6378137, differ than defined
 * in GLO ICD, refer A.3.1.2. */
#define GLO_A_E 6378136.0

/** Second zonal harmonic of the geopotential */
#define GLO_J02 1.08262575e-3

/** Earth's Gravitational Constant as defined in the GLO ICD in m^3 / s^2 */
#define GLO_GM 3.986004418e14

/** Earth's rotation rate as defined in the GLO ICD in rad / s */
#define GLO_OMEGAE_DOT 7.292115e-5

/** Frequency range between two adjacent GLO channel in Hz for L1 band*/
#define GLO_L1_DELTA_HZ 5.625e5

/** Frequency range between two adjacent GLO channel in Hz for L2 band */
#define GLO_L2_DELTA_HZ 4.375e5

/** GLO C/A code chipping rate in Hz. */
#define GLO_CA_CHIPPING_RATE .511e6

/** GLONASS CA chips number */
#define GLO_CA_CHIPS_NUM 511

/** GLO L1 carrier freq / code chipping rate
 * \note this is GLO_L1_HZ / GLO_CHIPPING_RATE */
#define GLO_L1_CARR_TO_CODE(fcn) \
  ((GLO_L1_HZ + (fcn)*GLO_L1_DELTA_HZ) / GLO_CA_CHIPPING_RATE)

/** GLO L2 carrier freq / code chipping rate
 * \note this is GLO_L2_HZ / GLO_CHIPPING_RATE */
#define GLO_L2_CARR_TO_CODE(fcn) \
  ((GLO_L2_HZ + (fcn)*GLO_L2_DELTA_HZ) / GLO_CA_CHIPPING_RATE)

/** GLO L1 maximum Doppler value [Hz],
 * refer to https://swiftnav.hackpad.com/Glonass-Dopplers-1ojKWwf8UN4 */
#define GLO_L1_DOPPLER_MAX_HZ (+4820.f)

/** GLO L2 maximum Doppler value [Hz] */
#define GLO_L2_DOPPLER_MAX_HZ (GLO_L1_DOPPLER_MAX_HZ * GLO_L2_HZ / GLO_L1_HZ)

/** \} */

/** \defgroup sbas_constants SBAS
 * Constants related to the SBAS system.
 * \{ */

/** Centre frequency of SBAS L1 */
#define SBAS_L1_HZ (1.023e6 * 1540)

/** Centre frequency of SBAS L5 */
#define SBAS_L5_HZ (1.023e6 * 1150)

/** Wavelength of SBAS L1 */
#define SBAS_L1_LAMBDA (GPS_C / SBAS_L1_HZ)

/** Wavelength of SBAS L5 */
#define SBAS_L5_LAMBDA (GPS_C / SBAS_L5_HZ)

/** SBAS L1 chip rate */
#define SBAS_L1CA_CHIPPING_RATE 1.023e6

/** SBAS L5 chip rate ??? */
#define SBAS_L5_CHIPPING_RATE 10.23e6

/** SBAS L1 carrier to code ratio */
#define SBAS_L1CA_CARR_TO_CODE (SBAS_L1_HZ / SBAS_L1CA_CHIPPING_RATE)

/** SBAS L5 carrier to code ratio */
#define SBAS_L5_CARR_TO_CODE (SBAS_L5_HZ / SBAS_L5_CHIPPING_RATE)

/** Number of chips in SBAS L1 signal */
#define SBAS_L1CA_CHIPS_NUM 1023

/** Number of chips in SBAS L5 signal ??? */
#define SBAS_L5_CHIPS_NUM 10230

/** SBAS L1 PRN period in ms */
#define SBAS_L1CA_PRN_PERIOD_MS 1

/** SBAS L5 PRN period in ms */
#define SBAS_L5_PRN_PERIOD_MS 1

/** SBAS L1 bit length [ms] */
#define SBAS_L1CA_BIT_LENGTH_MS 4

/** SBAS L1 pseudosymbol length [ms] */
#define SBAS_L1CA_PSYMBOL_LENGTH_MS 1

/** SBAS L1 maximum Doppler value [Hz], see SDCM ICD section 5.2.7
 * http://www.sdcm.ru/smglo/ICD_SDCM_1dot0_Eng.pdf */
#define SBAS_L1_DOPPLER_MAX_HZ (+210.f)

/** SBAS L5 maximum Doppler value [Hz] */
#define SBAS_L5_DOPPLER_MAX_HZ \
  (SBAS_L1_DOPPLER_MAX_HZ * SBAS_L5_HZ / SBAS_L1_HZ)

/** \} */

/** \defgroup beidou2_constants Beidou2
 * Constants related to the Beidou2 system.
 * See Beidou ICD 1.2.
 * \{ */

/** Centre frequency of Beidou2 B11 */
#define BDS2_B11_HZ (1.023e6 * (1540 - 14))

/** Centre frequency of Beidou2 B2 */
#define BDS2_B2_HZ (1.023e6 * 1180)

/** Centre frequency of Beidou3 B1C*/
#define BDS3_B1C_HZ (154 * 10.23e6)

/** Centre frequency of Beidou3 B3 */
#define BDS3_B3_HZ (124 * 10.23e6)

/** Centre frequency of Beidou3 B2b */
#define BDS3_B7_HZ (118 * 10.23e6)

/** Centre frequency of Beidou3 B2a */
#define BDS3_B5_HZ (115 * 10.23e6)

/** Wavelength of Beidou2 B11 */
#define BDS2_B11_LAMBDA (GPS_C / BDS2_B11_HZ)

/** Wavelength of Beidou2 B11 */
#define BDS2_B2_LAMBDA (GPS_C / BDS2_B2_HZ)

/** Beidou2 B11 chip rate (data) */
#define BDS2_B11_CHIPPING_RATE (2 * 1.023e6)

/** Beidou2 B2 chip rate (data) */
#define BDS2_B2_CHIPPING_RATE (2 * 1.023e6)

/** Beidou3 B1C chip rate */
#define BDS3_B1C_CHIPPING_RATE (1.023e6)

/** Beidou3 B3 chip rate */
#define BDS3_B3_CHIPPING_RATE (10 * 1.023e6)

/** Beidou3 B7 chip rate */
#define BDS3_B7_CHIPPING_RATE (10 * 1.023e6)

/** Beidou3 B2a chip rate */
#define BDS3_B5_CHIPPING_RATE (10 * 1.023e6)

/** Beidou2 B11 carrier to code ratio */
#define BDS2_B11_CARR_TO_CODE (BDS2_B11_HZ / BDS2_B11_CHIPPING_RATE)

/** Beidou2 B2 carrier to code ratio */
#define BDS2_B2_CARR_TO_CODE (BDS2_B2_HZ / BDS2_B2_CHIPPING_RATE)

/** Beidou3 B1C carrier to code ratio */
#define BDS3_B1C_CARR_TO_CODE (BDS3_B1C_HZ / BDS3_B1C_CHIPPING_RATE)

/** Beidou3 B3 carrier to code ratio */
#define BDS3_B3_CARR_TO_CODE (BDS3_B3_HZ / BDS3_B3_CHIPPING_RATE)

/** Beidou3 B2b carrier to code ratio */
#define BDS3_B7_CARR_TO_CODE (BDS3_B7_HZ / BDS3_B7_CHIPPING_RATE)

/** Beidou3 B2a carrier to code ratio */
#define BDS3_B5_CARR_TO_CODE (BDS3_B5_HZ / BDS3_B5_CHIPPING_RATE)

/** Number of chips in Beidou2 B11 (data) */
#define BDS2_B11_CHIPS_NUM 2046

/** Number of chips in Beidou2 B2 (data) */
#define BDS2_B2_CHIPS_NUM 2046

/** Number of chips in Beidou3 B1C */
#define BDS3_B1C_CHIPS_NUM 10230

/** Number of chips in Beidou3 B3 */
#define BDS3_B3_CHIPS_NUM 10230

/** Number of chips in Beidou3 B2b */
#define BDS3_B7_CHIPS_NUM 10230

/** Number of chips in Beidou3 B2a */
#define BDS3_B5_CHIPS_NUM 10230

/** Beidou2 B11 PRN code period */
#define BDS2_B11_PRN_PERIOD_MS 1

/** Beidou2 B2 PRN code period */
#define BDS2_B2_PRN_PERIOD_MS 1

/** Beidou3 B1C PRN code period */
#define BDS3_B1C_PRN_PERIOD_MS 10

/** Beidou3 B3 PRN code period */
#define BDS3_B3_PRN_PERIOD_MS 1

/** Beidou3 B2b PRN code period */
#define BDS3_B7_PRN_PERIOD_MS 1

/** Beidou3 B2a PRN code period */
#define BDS3_B5_PRN_PERIOD_MS 1

/** Beidou2 B11 (data) bit length */
#define BDS2_B11_BIT_LENGTH_MS 20

/** Beidou2 B11 (data) symbol length */
#define BDS2_B11_SYMB_LENGTH_MS 1

/** Beidou3 B2aI (data) symbol length */
#define BDS3_B5_SYMB_LENGTH_MS 1

/** Beidou2 B1 maximum Doppler value [Hz] */
#define BDS2_B11_DOPPLER_MAX_HZ (+4200.f)

/** Beidou2 B2 maximum Doppler value [Hz] */
#define BDS2_B2_DOPPLER_MAX_HZ \
  (BDS2_B11_DOPPLER_MAX_HZ * BDS2_B2_HZ / BDS2_B11_HZ)

/** Beidou3 B1C maximum Doppler value [Hz] */
#define BDS3_B1C_DOPPLER_MAX_HZ (BDS2_B11_DOPPLER_MAX_HZ)

/** Beidou3 B3 maximum Doppler value [Hz] */
#define BDS3_B3_DOPPLER_MAX_HZ \
  (BDS2_B11_DOPPLER_MAX_HZ * BDS3_B3_HZ / BDS2_B11_HZ)

/** Beidou3 B2b maximum Doppler value [Hz] */
#define BDS3_B7_DOPPLER_MAX_HZ \
  (BDS2_B11_DOPPLER_MAX_HZ * BDS3_B7_HZ / BDS2_B11_HZ)

/** Beidou3 B2a maximum Doppler value [Hz] */
#define BDS3_B5_DOPPLER_MAX_HZ \
  (BDS2_B11_DOPPLER_MAX_HZ * BDS3_B5_HZ / BDS2_B11_HZ)

/** Earth's gravitational constant [m^3 / s^2] */
#define BDS2_GM 3.986004418e14
/** Earth's rotation rate [rad / s] */
#define BDS2_OMEGAE_DOT 7.2921150e-5

/** \} */

/** \defgroup galileo_constants Galileo
 * Constants related to the Galileo system.
 * \{ */

/** Centre frequency of Galileo E1 */
#define GAL_E1_HZ (1.023e6 * 1540)

/** Centre frequency of Galileo E6 */
#define GAL_E6_HZ (1.023e6 * 1250)

/** Centre frequency of Galileo E5b */
#define GAL_E7_HZ (1.023e6 * 1180)

/** Centre frequency of Galileo E5AltBOC */
#define GAL_E8_HZ (1.023e6 * 1165)

/** Centre frequency of Galileo E5a */
#define GAL_E5_HZ (1.023e6 * 1150)

/** Wavelength of Galileo E1 */
#define GAL_E1_LAMBDA (GPS_C / GAL_E1_HZ)

/** Wavelength of Galileo E6 */
#define GAL_E6_LAMBDA (GPS_C / GAL_E6_HZ)

/** Wavelength of Galileo E5b */
#define GAL_E7_LAMBDA (GPS_C / GAL_E7_HZ)

/** Wavelength of Galileo E5AltBOC */
#define GAL_E8_LAMBDA (GPS_C / GAL_E8_HZ)

/** Wavelength of Galileo E5a */
#define GAL_E5_LAMBDA (GPS_C / GAL_E5_HZ)

/** Galileo E1 chip rate (data and pilot) */
#define GAL_E1_CHIPPING_RATE (1 * 1.023e6)

/** Galileo E6 chip rate (data and pilot) */
#define GAL_E6_CHIPPING_RATE (5 * 1.023e6)

/** Galileo E5b chip rate (data and pilot) */
#define GAL_E7_CHIPPING_RATE (10 * 1.023e6)

/** Galileo E5a chip rate (data and pilot) */
#define GAL_E5_CHIPPING_RATE (10 * 1.023e6)

/** Galileo E1 carrier to code ratio */
#define GAL_E1_CARR_TO_CODE (GAL_E1_HZ / GAL_E1_CHIPPING_RATE)

/** Galileo E6 carrier to code ratio */
#define GAL_E6_CARR_TO_CODE (GAL_E6_HZ / GAL_E6_CHIPPING_RATE)

/** Galileo E5b carrier to code ratio */
#define GAL_E7_CARR_TO_CODE (GAL_E7_HZ / GAL_E7_CHIPPING_RATE)

/** Galileo E5a carrier to code ratio */
#define GAL_E5_CARR_TO_CODE (GAL_E5_HZ / GAL_E5_CHIPPING_RATE)

/** Number of chips in Galileo E1B (data) */
#define GAL_E1B_CHIPS_NUM (4 * 1023)

/** Number of chips in Galileo E1C (pilot) */
#define GAL_E1C_CHIPS_NUM (4 * 1023)

/** Number of chips in Galileo E6 (data and pilot) */
#define GAL_E6_CHIPS_NUM (5 * 1023)

/** Number of chips in Galileo E5b (data and pilot) */
#define GAL_E7_CHIPS_NUM (10 * 1023)

/** Number of chips in Galileo E5a (data and pilot) */
#define GAL_E5_CHIPS_NUM (10 * 1023)

/** Galileo E1B code period */
#define GAL_E1B_PRN_PERIOD_MS 4

/** Galileo E1C code period */
#define GAL_E1C_PRN_PERIOD_MS 4

/** Galileo E6B code period */
#define GAL_E6B_PRN_PERIOD_MS 1

/** Galileo E6C code period */
#define GAL_E6C_PRN_PERIOD_MS 1

/** Galileo E5bI code period */
#define GAL_E7I_PRN_PERIOD_MS 1

/** Galileo E5bQ code period */
#define GAL_E7Q_PRN_PERIOD_MS 1

/** Galileo E5aI code period */
#define GAL_E5I_PRN_PERIOD_MS 1

/** Galileo E5aQ code period */
#define GAL_E5Q_PRN_PERIOD_MS 1

/** Galileo E1 maximum Doppler value [Hz] */
#define GAL_E1_DOPPLER_MAX_HZ (4000.f)

/** Galileo E6 maximum Doppler value [Hz] */
#define GAL_E6_DOPPLER_MAX_HZ (GAL_E1_DOPPLER_MAX_HZ * GAL_E6_HZ / GAL_E1_HZ)

/** Galileo E5b maximum Doppler value [Hz] */
#define GAL_E7_DOPPLER_MAX_HZ (GAL_E1_DOPPLER_MAX_HZ * GAL_E7_HZ / GAL_E1_HZ)

/** Galileo E5AltBOC maximum Doppler value [Hz] */
#define GAL_E8_DOPPLER_MAX_HZ (GAL_E1_DOPPLER_MAX_HZ * GAL_E8_HZ / GAL_E1_HZ)

/** Galileo E5a maximum Doppler value [Hz] */
#define GAL_E5_DOPPLER_MAX_HZ (GAL_E1_DOPPLER_MAX_HZ * GAL_E5_HZ / GAL_E1_HZ)

/** Earth's gravitational constant [m^3 / s^2] */
#define GAL_GM 3.986004418e14

/** \} */

/** \defgroup qzss_constants QZSS
 * Constants related to the QZSS system.
 * \{ */

/** Centre frequency of QZSS L1CA */
#define QZS_L1_HZ (1.023e6 * 1540)

/** Centre frequency of QZSS L2C */
#define QZS_L2_HZ (1.023e6 * 1200)

/** Centre frequency of QZSS L5 */
#define QZS_L5_HZ (1.023e6 * 1150)

/** Wavelength of QZSS L1CA */
#define QZS_L1_LAMBDA (GPS_C / QZS_L1_HZ)

/** Wavelength of QZSS L2 */
#define QZS_L2_LAMBDA (GPS_C / QZS_L2_HZ)

/** Wavelength of QZSS L5 */
#define QZS_L5_LAMBDA (GPS_C / QZS_L5_HZ)

/** QZSS L1C/A chip rate */
#define QZS_L1CA_CHIPPING_RATE (1 * 1.023e6)

/** QZSS L1C/A carrier to code ratio */
#define QZS_L1CA_CARR_TO_CODE (QZS_L1_HZ / QZS_L1CA_CHIPPING_RATE)

/** Number of chips in QZSS L1C/A signal */
#define QZS_L1CA_CHIPS_NUM (1023)

/** QZSS L2 CM chips number */
#define QZS_L2CM_CHIPS_NUM (10230 + 10230)

/** QZSS L2 CL chips number */
#define QZS_L2CL_CHIPS_NUM (767250 + 767250)

/** QZSS L2C carrier to code ratio */
#define QZS_L2C_CARR_TO_CODE (QZS_L2_HZ / QZS_L1CA_CHIPPING_RATE)

/** QZSS L1 C/A PRN period in ms */
#define QZS_L1CA_PRN_PERIOD_MS 1

/** QZSS L1 C/A bit length [ms] */
#define QZS_L1CA_BIT_LENGTH_MS 20

/** QZSS L1 C/A pseudosymbol length [ms] */
#define QZS_L1CA_PSYMBOL_LENGTH_MS 1

/** QZSS L2 CL PRN start interval in ms */
#define QZS_L2CL_PRN_START_INTERVAL_MS 20

/** QZSS L2 CL PRN chips per tracker start interval */
#define QZS_L2CL_PRN_CHIPS_PER_INTERVAL \
  (QZS_L2CL_CHIPS_NUM * QZS_L2CL_PRN_START_INTERVAL_MS / QZS_L2CL_PRN_PERIOD_MS)

/** Number of start points in QZSS L2 CL PRN */
#define QZS_L2CL_PRN_START_POINTS \
  (QZS_L2CL_PRN_PERIOD_MS / QZS_L2CL_PRN_START_INTERVAL_MS)

/** QZSS L2C CNAV message length [bits] */
#define QZS_CNAV_MSG_LENGTH 300

/** QZSS L1 maximum Doppler value [Hz] */
#define QZS_L1_DOPPLER_MAX_HZ (+1200.f)

/** QZSS L2 maximum Doppler value [Hz] */
#define QZS_L2_DOPPLER_MAX_HZ (QZS_L1_DOPPLER_MAX_HZ * QZS_L2_HZ / QZS_L1_HZ)

/** QZSS L5 maximum Doppler value [Hz] */
#define QZS_L5_DOPPLER_MAX_HZ (QZS_L1_DOPPLER_MAX_HZ * QZS_L5_HZ / QZS_L1_HZ)

/** \} */

/** \defgroup dgnss_constants DGNSS
 * Approximate variance values used by the KF and IAR hypothesis test.
 * \{ */

/** The default DD carrier phase variance to use in the hypothesis testing. */
#define DEFAULT_PHASE_VAR_TEST (9e-4 * 16)
/** The default DD pseudorange variance to use in the hypothesis testing. */
#define DEFAULT_CODE_VAR_TEST (100 * 400)
/** The default DD carrier phase variance to use in the Kalman filter. */
#define DEFAULT_PHASE_VAR_KF (9e-4 * 16)
/** The default DD pseudorange variance to use in the Kalman filter. */
#define DEFAULT_CODE_VAR_KF (100 * 400)
/** The default variance of the process noise Kalman filter. Its particular use
 * is different from that of a normal KF process noise. It's still a random
 * walk, but in a special space. Look at the code for its current usage.*/
#define DEFAULT_AMB_DRIFT_VAR 1e-8
/** The variance with which to initialize the Kalman Filter. */
#define DEFAULT_AMB_INIT_VAR 1e25
/** The variance with which to add new sats to the Kalman Filter.
 * TODO deprecate in lieu of amb_init_var once we do some tuning. */
#define DEFAULT_NEW_INT_VAR 1e25

#define GME 3.986004415E+14 /* earth gravitational constant */
#define GMS 1.327124E+20    /* sun gravitational constant */
#define GMM 4.902801E+12    /* moon gravitational constant */

#define SPP_BASE_STATION_DIFFERENCE_WARNING_THRESHOLD 50.0
#define SPP_BASE_STATION_DIFFERENCE_RESET_THRESHOLD 1000.0
#define SURVEYED_BASE_STATION_MOVEMENT_RESET_THRESHOLD 1e-3

#define FILENAME_SIZE 256

/** \} */

/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_CONSTANTS_H */
