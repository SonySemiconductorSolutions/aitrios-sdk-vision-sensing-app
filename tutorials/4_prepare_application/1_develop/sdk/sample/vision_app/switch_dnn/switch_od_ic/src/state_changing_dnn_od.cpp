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

#include "state_changing_dnn_od.h"
#include "defines.h"

StateChangingDnnOd::StateChangingDnnOd(EvpController* evp,
        EvpThreadController* evp_thread,
        SessController* sess,
        SessAllocator* sess_allocator,
        SenscordController* senscord,
        AnalyzerCommon* analyzer_com,
        AnalyzerOd* analyzer_od,
        AnalyzerIc* analyzer_ic,
        DeviceModel* device_model,
        AppModel* app_model) : analyzer_od_(analyzer_od), analyzer_ic_(analyzer_ic),
        StateBase(evp, evp_thread, sess, sess_allocator, senscord, analyzer_com, device_model, app_model) {
    own_state_ = SequenceState::kChangingDnnOd;
    next_state_ = SequenceState::kOd;
}

StateChangingDnnOd::~StateChangingDnnOd() {}

AppState::SequenceStatus StateChangingDnnOd::VerifyDnnStream() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnStream(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t target_network_id;
    const struct senscord_dnn_property* dnn_property = app_model_->NextDnnProperty();
    if (dnn_property == nullptr) {
        // abnormal
        return SequenceStatus::kErrorChangeToOd;
    }
    target_network_id = dnn_property->network_id;

    uint32_t od_network_id;
    analyzer_od_->GetNetworkId(od_network_id);

    // Get stream property
    const struct senscord_dnn_property stream_dnn = device_model_->DnnStream();

    if (stream_dnn.network_id != target_network_id) {
        // If the expected AI model is not set, set dnn and continue

        if (stream_dnn.network_id == od_network_id) {
            // This may occurred reset by stop, start inference.
            // Change state to OD.
            app_model_->ClearNextDnnProperty();
            return SequenceStatus::kErrorChangeToOd;
        }

        // This means previous set dnn failed, so retry set dnn
        return SequenceStatus::kRetrySetProperty;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateChangingDnnOd::RetrySetDnn() {
    const struct senscord_dnn_property* dnn_property = app_model_->NextDnnProperty();
    if (dnn_property == nullptr) {
        // abnormal
        return SequenceStatus::kErrorChangeToOd;
    }

    struct senscord_status_t status;
    int32_t ret = senscord_->SetDnn(dnn_property, status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateChangingDnnOd::VerifyDnnChannel() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnChannel(), device_model_->Channel(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t target_network_id;
    const struct senscord_dnn_property* dnn_property = app_model_->NextDnnProperty();
    if (dnn_property == nullptr) {
        // abnormal
        return SequenceStatus::kErrorChangeToOd;
    }
    target_network_id = dnn_property->network_id;

    // Get channel property
    const struct senscord_dnn_property channel_dnn = device_model_->DnnChannel();
    if (channel_dnn.network_id != target_network_id) {
        // If the expected AI model is not set, continue
        return SequenceStatus::kContinue;
    }

    app_model_->ClearNextDnnProperty();
    return SequenceStatus::kSuccessCompleted;
}
