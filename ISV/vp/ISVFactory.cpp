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

#include "ISVFactory.h"
#include "ISVProcessor.h"
#include "manager.h"

namespace intel {
namespace isv {

ISVFactory::ISVFactory(
        ISVProcessorCallBack* callback)
    :   mCallBack(callback)
{
}

ISVFactory::~ISVFactory()
{
    mCallBack = NULL;
}

IISVProcessor* ISVFactory::createVpProcessor()
{
    if (mCallBack == NULL)
        return new ISVProcessor(IISVProcessor::ISV_SYNC_MODE, NULL);
    else
        return new ISVProcessor(IISVProcessor::ISV_ASYNC_MODE, mCallBack);
}

IISVPolicyManager* ISVFactory::createPolicyManager()
{
        return new ISVPolicyManager();
}
} // namespace intel
} // namespace isv

