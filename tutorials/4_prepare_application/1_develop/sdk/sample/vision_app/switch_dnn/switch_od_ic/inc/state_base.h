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

#ifndef _STATE_BASE_H_
#define _STATE_BASE_H_

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
 * @class StateBase
 * @brief Base app sequence implementation class
 */
class StateBase : public AppState {
private:
    EvpController* evp_;
    EvpThreadController* evp_thread_;
    AnalyzerCommon* analyzer_com_;
protected:
    SessController* sess_;
    SessAllocator* sess_allocator_;
    SenscordController* senscord_;
    DeviceModel* device_model_;
    AppModel* app_model_;
protected:
    SequenceState own_state_;
    SequenceState next_state_;

public:
    /**
     * @brief Constructor
     */
    StateBase(EvpController* evp,
        EvpThreadController* evp_thread,
        SessController* sess,
        SessAllocator* sess_allocator,
        SenscordController* senscord,
        AnalyzerCommon* analyzer_com,
        DeviceModel* device_model,
        AppModel* app_model);

    /**
     * @brief Destructor
     */
    virtual ~StateBase();

    SequenceState OwnState() const override;

    /**
     * @brief Get frame
     * @return SequenceStatus
     */
    virtual SequenceStatus GetFrame() override;

    /**
     * @brief Verify dnn stream wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnStream() override;

    /**
     * @brief Retry set dnn wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus RetrySetDnn() override;

    /**
     * @brief Get channel
     * @return SequenceStatus
     */
    virtual SequenceStatus GetChannel() override;

    /**
     * @brief Verify dnn channel wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnChannel() override;

    /**
     * @brief Get crop property wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus GetCropProperty() override;

    /**
     * @brief Set crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SetCrop() override;

    /**
     * @brief Verify crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyCrop() override;

    /**
     * @brief Retry set crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus RetrySetCrop() override;

    /**
     * @brief Get raw data wrapper
     * @param [out] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus GetRawData(struct senscord_raw_data_t& raw_data) override;

    /**
     * @brief Analyze data wrapper
     * @param [in] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus AnalyzeData(struct senscord_raw_data_t& raw_data) override;

    /**
     * @brief Save crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SaveCrop() override;

    /**
     * @brief Serialize data wrapper
     * @param [out] p_out_buf serialized data
     * @param [out] out_size serialized data size
     * @return SequenceStatus
     */
    virtual SequenceStatus SerializeData(void** p_out_buf, uint32_t& out_size) override;

    /**
     * @brief Send serialize data wrapper
     * @param [in] p_out_buf serialized data
     * @param [in] out_size serialized data size
     * @param [in] timestamp timestamp
     * @return SequenceStatus
     */
    virtual SequenceStatus SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) override;

    /**
     * @brief Set dnn wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SetDnn() override;

    /**
     * @brief Release frame
     */
    void ReleaseFrame() override;

public:
    /**
     * @brief Get next state
     * @return SequenceState
     */
    virtual SequenceState NextState() override;

protected:
    /**
     * @brief Set next state
     * @param [in] SequenceState state
     */
    virtual void SetNextState(SequenceState state) override;
};

#endif // _STATE_BASE_H_
