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

#ifndef _APP_SEQUENCE_H_
#define _APP_SEQUENCE_H_

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

#include "controllers.h"
#include "device_model.h"
#include "app_model.h"
#include "app_context.h"
#include "app_state.h"

/**
 * @class AppSequence
 * @brief App sequencer class
 */
class AppSequence {
public:
    /**
     * @brief Constructor
     */
    AppSequence();

    /**
     * @brief Destructor
     */
    virtual ~AppSequence();

    /**
     * @brief Initialize app sequencer
     * @return int : 0 is success, negative value is failure
     */
    int32_t Initialize();

    /**
     * @brief App sequencer main process
     * @return int : 0 is success, negative value is failure
     */
    int32_t MainLoop();

    /**
     * @brief Finalize app sequencer
     * @return int : 0 is success, negative value is failure
     */
    int32_t Finalize();

private:
    /**
     * @brief Validate parameter
     * @param [in] param parameter
     * @return int : 0 is success, positive value is failure
     */
    int ValidateParam(const void* param);

    /**
     * @brief Clear parameter
     */
    void ClearValidatingParam();

    /**
     * @brief Set parameter
     * @param [in] param parameter
     */
    void SetValidatedParam(const void* param);

    /**
     * @brief Release frame
     * @param [in] AppContext context
     * @param [in] SequenceStatus status
     */
    void ReleaseFrameForContinueOrBreak(AppContext& context, AppState::SequenceStatus status);

private:
    EvpController* evp_;
    EvpThreadController* evp_thread_;
    SessAllocator* sess_allocator_;
    SessController* sess_;
    SenscordController* senscord_;

    AnalyzerCommon* analyzer_com_;
    AnalyzerOd* analyzer_od_;
    AnalyzerIc* analyzer_ic_;

    std::vector<AnalyzerBase*> analyzers_;

    DeviceModel* device_model_;
    AppModel* app_model_;

    bool fatal_error_;
};

#endif // _APP_SEQUENCE_H_
