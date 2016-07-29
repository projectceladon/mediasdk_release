/*
 * Copyright (C) 2012 Intel Corporation.  All rights reserved.
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

#ifndef __ISV_FACTORY_H
#define __ISV_FACTORY_H

#include "ISVManager.h"
#include "ISVVp.h"

namespace intel {
namespace isv {

class IISVFactory
{
public:
    IISVFactory() {}
    virtual ~IISVFactory() {}
public:
    virtual IISVProcessor* createVpProcessor() = 0;
    virtual IISVPolicyManager* createPolicyManager() = 0;
};

class ISVFactory : public IISVFactory
{
public:
    ISVFactory(ISVProcessorCallBack* callback);
    virtual ~ISVFactory();
public:
    IISVProcessor* createVpProcessor();
    IISVPolicyManager* createPolicyManager();

private:
    // vp callback
    ISVProcessorCallBack* mCallBack;
};

} // namespace intel
} // namespace isv

#endif // __ISV_FACTORY_H
