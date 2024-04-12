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

#ifndef _APP_STATE_H_
#define _APP_STATE_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

#include "controllers.h"
#include "device_model.h"
#include "app_model.h"

/**
 * @enum AppSequence::SequenceState
 * @brief The state of sequence
 */
enum class SequenceState {
    kOd = 0,
    kChangingDnnIc,
    kChangingCropIc,
    kIc,
    kChangingDnnOd,
    // kChangingOdCrop, // first AI Model's crop is set by set dnn because set dnn reset crop value to command parameter file's crop setting.
    kNumberOfSequenceState
};

/**
 * @class AppState
 * @brief Base app sequence interface class
 */
class AppState {
public:
    /**
     * @enum BaseImpl::SequenceStatus
     * @brief The status code of app sequence implementation
     */
    enum class SequenceStatus {
        /** Nothing to do */
        kNop = 0,

        /** Continue main loop */
        kContinue,

        /** Need to retry set property */
        kRetrySetProperty,

        /** Success */
        kSuccess,
        /** Success and current state completed */
        kSuccessCompleted,

        /** Error */
        kError,
        /** Error and reset to default Od state */
        kErrorChangeToOd,

        /** Fatal error and finish main loop */
        kErrorFatal,
    };

public:
    /**
     * @brief Constructor
     */
    AppState();

    /**
     * @brief Destructor
     */
    virtual ~AppState();

    /**
     * @brief Get own state
     * @return SequenceState
     */
    virtual SequenceState OwnState() const = 0;

public:
    /**
     * @brief Get frame
     * @return SequenceStatus
     */
    virtual SequenceStatus GetFrame() = 0;

    /**
     * @brief Verify dnn stream wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnStream() = 0;

    /**
     * @brief Retry set dnn wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus RetrySetDnn() = 0;

    /**
     * @brief Get channel
     * @return SequenceStatus
     */
    virtual SequenceStatus GetChannel() = 0;

    /**
     * @brief Verify dnn channel wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyDnnChannel() = 0;

    /**
     * @brief Get crop property wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus GetCropProperty() = 0;

    /**
     * @brief Set crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SetCrop() = 0;

    /**
     * @brief Verify crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus VerifyCrop() = 0;

    /**
     * @brief Retry set crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus RetrySetCrop() = 0;

    /**
     * @brief Get raw data wrapper
     * @param [out] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus GetRawData(struct senscord_raw_data_t& raw_data) = 0;

    /**
     * @brief Analyze data wrapper
     * @param [in] raw_data raw data
     * @return SequenceStatus
     */
    virtual SequenceStatus AnalyzeData(struct senscord_raw_data_t& raw_data) = 0;

    /**
     * @brief Save crop wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SaveCrop() = 0;

    /**
     * @brief Serialize data wrapper
     * @param [out] p_out_buf serialized data
     * @param [out] out_size serialized data size
     * @return SequenceStatus
     */
    virtual SequenceStatus SerializeData(void** p_out_buf, uint32_t& out_size) = 0;

    /**
     * @brief Send serialize data wrapper
     * @param [in] p_out_buf serialized data
     * @param [in] out_size serialized data size
     * @param [in] timestamp timestamp
     * @return SequenceStatus
     */
    virtual SequenceStatus SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) = 0;

    /**
     * @brief Set dnn wrapper
     * @return SequenceStatus
     */
    virtual SequenceStatus SetDnn() = 0;

    /**
     * @brief Release frame
     */
    virtual void ReleaseFrame() = 0;

    /**
     * @brief Get next state
     * @return SequenceState
     */
    virtual SequenceState NextState() = 0;

protected:
    /**
     * @brief Set next state
     * @param [in] SequenceState state
     */
    virtual void SetNextState(SequenceState state) = 0;
};

#endif // _APP_STATE_H_
