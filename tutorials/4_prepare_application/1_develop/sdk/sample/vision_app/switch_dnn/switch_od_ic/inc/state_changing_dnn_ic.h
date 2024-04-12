/****************************************************************************
 * Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 ****************************************************************************/

#ifndef _STATE_CHANGING_DNN_IC_H_
#define _STATE_CHANGING_DNN_IC_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

#include "controllers.h"
#include "state_base.h"

/**
 * @class StateChangingDnnIc
 * @brief Implementation class of changing ic dnn state
 */
class StateChangingDnnIc: public StateBase {
public:
    /**
     * @brief Constructor
     */
    StateChangingDnnIc(EvpController* evp,
            EvpThreadController* evp_thread,
            SessController* sess,
            SessAllocator* sess_allocator,
            SenscordController* senscord,
            AnalyzerCommon* analyzer_com,
            AnalyzerOd* analyzer_od,
            AnalyzerIc* analyzer_ic,
            DeviceModel* device_model,
            AppModel* app_model);

    /**
     * @brief Destructor
     */
    virtual ~StateChangingDnnIc();

    /**
     * @brief Verify dnn stream
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnStream() override;

    /**
     * @brief Retry set dnn
     * @return SequenceStatus
     */
    virtual SequenceStatus RetrySetDnn() override;

    /**
     * @brief Verify dnn channel
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnChannel() override;

    /**
     * @brief Set crop
     * @return SequenceStatus
     */
    virtual SequenceStatus SetCrop() override;

private:
    AnalyzerOd* analyzer_od_;
    AnalyzerIc* analyzer_ic_;
};

#endif // _STATE_CHANGING_DNN_IC_H_
