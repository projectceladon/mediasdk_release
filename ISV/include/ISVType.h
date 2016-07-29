/*
 * Copyright (C) 2014 Intel Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __ISV_TYPE_H
#define __ISV_TYPE_H

#include <utils/RefBase.h>

namespace intel {
namespace isv {

#define ISV_UNUSED(x) ((void)&(x));

#define MAX_FILTER_PARAMETERS (32)
//FIXME: copy from OMX_Core.h

/* interlaced frame flag: This flag is set to indicate the buffer contains a
 * top and bottom field and display ordering is top field first.
 * @ingroup buf
 */
#define OMX_BUFFERFLAG_TFF 0x00010000

/* interlaced frame flag: This flag is set to indicate the buffer contains a
 * top and bottom field and display ordering is bottom field first.
 * @ingroup buf
 */
#define OMX_BUFFERFLAG_BFF 0x00020000

/** \brief Video filter types */
typedef enum _ISVFilterType {
    ISVFilterNone = 0,
    ISVFilterNoiseReduction,
    ISVFilterDeinterlacing,
    ISVFilterSharpening,
    ISVFilterColorBalance,
    ISVFilterDeblocking,
    ISVFilterFrameRateConversion,
    ISVFilterSkinToneEnhancement,
    ISVFilterTotalColorCorrection,
    ISVFilterNonLinearAnamorphicScaling,
    ISVFilterImageStabilization,
    ISVFilterCount
} ISVFilterType;

typedef struct _ISVFilterValueRange {
    /** \brief Minimum value supported, inclusive. */
    float               min_value;
    /** \brief Maximum value supported, inclusive. */
    float               max_value;
    /** \brief Default value. */
    float               default_value;
    /** \brief Step value that alters the filter behaviour in a sensible way. */
    float               step;
} ISVFilterValueRange;

typedef struct _ISVFilterAlgorithmRange {
    /** \brief Minimum algorithm supported, inclusive. */
    int               min_algo;
    /** \brief Maximum algorithm supported, inclusive. */
    int               max_algo;
    /** \brief Default algorithm. */
    int               default_algo;
    /** \brief Step value that alters the filter behaviour in a sensible way. */
    int               step;
} ISVFilterAlgorithmRange;

typedef struct _ISVFilterCapability {
    /** \brief Filter type */
    ISVFilterType           type;
    /** \brief Enable status */
    bool                    enabled;
    /** \brief supported min resolution */
    uint32_t                minResolution;
    /** \brief supported max resolution */
    uint32_t                maxResolution;
    /** \brief Filter algorithm if defined */
    ISVFilterAlgorithmRange algorithm;
    /** \brief parameters' array */
    ISVFilterValueRange     parameters[MAX_FILTER_PARAMETERS];
    /** \brief the number of parameter array */
    uint32_t            numberParameter;
} ISVFilterCapability;

typedef struct _ISVFilterParameter {
    /** \brief Filter type */
    ISVFilterType  filter;
    /** \brief the values of parameter */
    float          values[MAX_FILTER_PARAMETERS];
    /** \brief algorithm if defined */
    int            algorithm;
} ISVFilterParameter;

typedef enum {
    kPortIndexInput  = 0,
    kPortIndexOutput = 1
}PORT_INDEX;

/** \brief the FRC rate */
typedef enum _FRC_RATE {
    FRC_RATE_1X   =  0,
    FRC_RATE_2X   =  1,
    FRC_RATE_2_5X =  2,
    FRC_RATE_4X   =  4
} ISV_FRC_RATE;

typedef enum
{
    DEINTERLACE_NONE  = 0,
    DEINTERLACE_BOB   = 1,                   // BOB DI
    DEINTERLACE_WEAVE = 2,                   // WEAVE
    DEINTERLACE_ADI   = 3,                   // Adaptive DI
    DEINTERLACE_CDI   = 4,                   // Compensated DI
} deinterlace_t;

typedef enum {
    VPP_COMMON_ON   = 1,        // VPP is on
    VPP_FRC_ON      = 1 << 1,   // FRC is on
} VPP_SETTING_STATUS;

} // namespace intel
} // namespace isv

#endif /* __ISV_TYPE_H */
