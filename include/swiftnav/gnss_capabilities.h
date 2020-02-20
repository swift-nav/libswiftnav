/*
 * Copyright (C) 2020 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_GNSS_CAPABILITIES_H
#define LIBSWIFTNAV_GNSS_CAPABILITIES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \defgroup constants GNSSS capabilities
 * \{ */

/*
 * Note: BDS GEO SVs are marked as inactive,
 * in order to prevent their acquisition.
 * Configuration compliant with to
 * http://www.csno-tarc.cn/en/system/constellation
 * retrieved on May 2nd
 */
#define GNSS_CAPB_BDS_ACTIVE ((u64)0x1fbffcbfe0)
#define GNSS_CAPB_BDS_D2NAV ((u64)0x000000001f)
#define GNSS_CAPB_BDS_B2 ((u64)0x000000bfff)
#define GNSS_CAPB_BDS_B2A ((u64)0x1fbffc0000)

/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_GNSS_CAPABILITIES_H */
