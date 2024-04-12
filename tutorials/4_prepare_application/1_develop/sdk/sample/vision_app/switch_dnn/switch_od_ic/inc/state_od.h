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

#ifndef _STATE_OD_H_
#define _STATE_OD_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "state_base.h"

/**
 * @class StateOd
 * @brief Implementation class of od state
 */
class StateOd: public StateBase {
public:
    /**
     * @brief Constructor
     */
    StateOd(EvpController* evp,
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
    virtual ~StateOd();

    /**
     * @brief Verify dnn  stream
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnStream() override;

    /**
     * @brief Verify dnn channel
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnChannel() override;

    /**
     * @brief Get crop property wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus GetCropProperty() override;

    /**
     * @brief Get raw data
     * @param [out] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus GetRawData(struct senscord_raw_data_t& raw_data) override;

    /**
     * @brief Analyze data
     * @param [in] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus AnalyzeData(struct senscord_raw_data_t& raw_data) override;

    /**
     * @brief Save crop
     * @return SequenceStatus
     */
    virtual SequenceStatus SaveCrop() override;

    /**
     * @brief Serialize data
     * @param [out] p_out_buf serialized data
     * @param [out] out_size serialized data size
     * @return SequenceStatus
     */
    virtual SequenceStatus SerializeData(void** p_out_buf, uint32_t& out_size) override;

    /**
     * @brief Send serialize data
     * @param [in] p_out_buf serialized data
     * @param [in] out_size serialized data size
     * @param [in] timestamp timestamp
     * @return SequenceStatus
     */
    virtual SequenceStatus SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) override;

    /**
     * @brief Set dnn
     * @return SequenceStatus
     */
    virtual SequenceStatus SetDnn() override;

private:
    AnalyzerOd* analyzer_od_;
    AnalyzerIc* analyzer_ic_;
};

#endif // _STATE_OD_H_
