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

#ifndef _APP_CONTEXT_H_
#define _APP_CONTEXT_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

#include "controllers.h"
#include "device_model.h"
#include "app_model.h"
#include "app_state.h"

/**
 * @class AppContext
 * @brief Base app context for state class
 */
class AppContext {
private:
    AppState *state_;

public:
    /**
     * @brief Constructor
     */
    explicit AppContext(EvpController* evp,
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
    virtual ~AppContext();

    /**
     * @brief Set state
     * @param [in] state raw state
     */
    void ChangeState(SequenceState state);

    /**
     * @brief Get state
     * @return AppState
     */
    AppState* State() const;
public:
    /**
     * @brief Get frame
     * @return SequenceStatusGetFrame
     */
    AppState::SequenceStatus GetFrame();

    /**
     * @brief Verify dnn stream wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus VerifyDnnStream();

    /**
     * @brief Retry set dnn wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus RetrySetDnn();

    /**
     * @brief Get channel
     * @return SequenceStatusGetChannel
     */
    AppState::SequenceStatus GetChannel();

    /**
     * @brief Verify dnn channel wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus VerifyDnnChannel();

    /**
     * @brief Get crop property wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus GetCropProperty();

    /**
     * @brief Set crop wrapper
     * @return SequenceStatusSetCrop
     */
    AppState::SequenceStatus SetCrop();

    /**
     * @brief Verify crop wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus VerifyCrop();

    /**
     * @brief Retry set crop wrapper
     * @return SequenceStatus
     */
    AppState::SequenceStatus RetrySetCrop();

    /**
     * @brief Get raw data wrapper
     * @param [out] raw_data raw data
     * @return SequenceStatusGetRawData
     */
    AppState::SequenceStatus GetRawData(struct senscord_raw_data_t& raw_data);

    /**
     * @brief Analyze data wrapper
     * @param [in] raw_data raw data
     * @return SequenceStatusAnalyze
     */
    AppState::SequenceStatus AnalyzeData(struct senscord_raw_data_t& raw_data);

    /**
     * @brief Save crop wrapper
     * @return SequenceStatusSaveCrop
     */
    AppState::SequenceStatus SaveCrop();

    /**
     * @brief Serialize data wrapper
     * @param [out] p_out_buf serialized data
     * @param [out] out_size serialized data size
     * @return SequenceStatusSerialize
     */
    AppState::SequenceStatus SerializeData(void** p_out_buf, uint32_t& out_size);

    /**
     * @brief Send serialize data wrapper
     * @param [in] p_out_buf serialized data
     * @param [in] out_size serialized data size
     * @param [in] timestamp timestamp
     * @return SequenceStatusSend
     */
    AppState::SequenceStatus SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp);

    /**
     * @brief Set dnn wrapper
     * @return SequenceStatusSetDnn
     */
    AppState::SequenceStatus SetDnn();

    /**
     * @brief Release frame
     */
    void ReleaseFrame();

    /**
     * @brief Get next state
     * @return SequenceState
     */
    SequenceState NextState();

private:
    AppState* impl_states_[static_cast<int>(SequenceState::kNumberOfSequenceState)] = {};
};

#endif // _APP_CONTEXT_H_
