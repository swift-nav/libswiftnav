#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <swiftnav/gnss_time.h>
#include <swiftnav/nav_meas.h>

namespace test_data {

constexpr uint16_t cTorWn = 1939;
constexpr double cTorTow = 42.0;

/* time of reception for the tests */
constexpr gps_time_t tor = {.tow = cTorTow, .wn = cTorWn};

constexpr navigation_measurement_t nm1 = {
    .raw_pseudorange = 23946993.888943646,
    .sat_pos = {-19477278.087422125, -7649508.9457812719, 16674633.163554827},
    .cn0 = 41.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 9, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm1_no_doppler = {
    .raw_pseudorange = 23946993.888943646,
    .sat_pos = {-19477278.087422125, -7649508.9457812719, 16674633.163554827},
    .cn0 = 39.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 9, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID};

constexpr navigation_measurement_t nm2 = {
    .raw_pseudorange = 22932174.156858064,
    .sat_pos = {-9680013.5408340245, -15286326.354385279, 19429449.383770257},
    .cn0 = 43.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 1, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm3 = {
    .raw_pseudorange = 24373231.648055989,
    .sat_pos = {-19858593.085281931, -3109845.8288993631, 17180320.439503901},
    .cn0 = 35.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 2, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm4 = {
    .raw_pseudorange = 24779663.252316438,
    .sat_pos = {6682497.8716542246, -14006962.389166718, 21410456.275678463},
    .cn0 = 27.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 3, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm5 = {
    .raw_pseudorange = 26948717.022331879,
    .sat_pos = {7415370.9916331079, -24974079.044485383, -3836019.0262199985},
    .cn0 = 39.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 4, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm6 = {
    .raw_pseudorange = 23327405.435463827,
    .sat_pos = {-2833466.1648670658, -22755197.793894723, 13160322.082875408},
    .cn0 = 38.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 5, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

/* doppler measurement broken */
constexpr navigation_measurement_t nm6b = {
    .raw_pseudorange = 23327405.435463827,
    .raw_measured_doppler = 10000, /* Doppler outlier */
    .sat_pos = {-2833466.1648670658, -22755197.793894723, 13160322.082875408},
    .sat_vel = {0, 0, 0},
    .cn0 = 40,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 5, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID | NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm7 = {
    .raw_pseudorange = 27371419.016328193,
    .sat_pos = {14881660.383624561, -5825253.4316490609, 21204679.68313824},
    .cn0 = 42.3,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 6, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm8 = {
    .raw_pseudorange = 26294221.697782904,
    .sat_pos = {12246530.477279386, -22184711.955107089, 7739084.2855069181},
    .cn0 = 45.1,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 7, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm9 = {
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .cn0 = 37.3,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 8, .code = CODE_GPS_L1CA},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm10 = {
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .cn0 = 41.0,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 8, .code = CODE_GPS_L2CM},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

/* broken measurement */
constexpr navigation_measurement_t nm10b = {
    .raw_pseudorange = 25781999.479948733 + 30000,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .cn0 = 39.2,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 8, .code = CODE_GPS_L2CM},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

constexpr navigation_measurement_t nm11 = {
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .cn0 = 45.4,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 11, .code = CODE_GPS_L2CM},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

// Note this is a copy of GPS nm1 but set to code GAL_E1B, do not combine
// them in the same test case
constexpr navigation_measurement_t gal_nm1 = {
    .raw_pseudorange = 23946993.888943646,
    .sat_pos = {-19477278.087422125, -7649508.9457812719, 16674633.163554827},
    .cn0 = 41.1,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 9, .code = CODE_GAL_E1B},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

// Note this is a copy of GPS nm2 but set to code GAL_E1B, do not combine
// them in the same test case
constexpr navigation_measurement_t gal_nm2 = {
    .raw_pseudorange = 22932174.156858064,
    .sat_pos = {-9680013.5408340245, -15286326.354385279, 19429449.383770257},
    .cn0 = 39.7,
    .lock_time = 5,
    .tot = {.tow = cTorTow - 0.077, .wn = cTorWn},
    .sid = {.sat = 1, .code = CODE_GAL_E1B},
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

}  // namespace test_data

#endif
