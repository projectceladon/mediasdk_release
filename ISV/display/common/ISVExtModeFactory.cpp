/*
 * Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
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
 * Author: tianyang.zhu@intel.com
 */

#include <ISVDisplay.h>
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
#include "ISVExtModeBaseOnPrivateService.h"
#elif defined( INTEL_FEATURE_VIDEO_EXT_MODE )
#include "ISVExtModeBaseOnHwcService.h"
#endif

namespace intel {
namespace isv {

ISVDisplay::IISVExtMode* ISVDisplay::ISVExtModeFactory::getExtMode() {
#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    return new ISVExtModeBaseOnPrivateService();
#elif defined( INTEL_FEATURE_VIDEO_EXT_MODE )
    return new ISVExtModeBaseOnHwcService();
#else
    return NULL;
#endif
}

} // namespace isv
} // namespace intel

