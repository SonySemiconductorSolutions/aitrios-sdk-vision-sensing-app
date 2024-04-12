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

#include "state_changing_crop_ic.h"
#include "defines.h"

StateChangingCropIc::StateChangingCropIc(EvpController* evp,
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
    own_state_ = SequenceState::kChangingCropIc;
    next_state_ = SequenceState::kIc;
}

StateChangingCropIc::~StateChangingCropIc() {}

AppState::SequenceStatus StateChangingCropIc::VerifyDnnStream() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnStream(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t ic_network_id;
    analyzer_ic_->GetNetworkId(ic_network_id);

    uint32_t od_network_id;
    analyzer_od_->GetNetworkId(od_network_id);

    // Get stream property
    const struct senscord_dnn_property stream_dnn = device_model_->DnnStream();

    if (stream_dnn.network_id != ic_network_id) {
        // If the expected AI model is not set, reset state

        if (stream_dnn.network_id != od_network_id) {
            // abnormal case
            ERR_PRINTF("device stream network_id %x is not ic_network_id %x nor od_network_id %x .",
                        stream_dnn.network_id, ic_network_id, od_network_id);
        }

        // This may occurred reset by stop, start inference
        app_model_->ClearNextCropProperty();
        return SequenceStatus::kErrorChangeToOd;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateChangingCropIc::VerifyDnnChannel() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnChannel(), device_model_->Channel(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t ic_network_id;
    analyzer_ic_->GetNetworkId(ic_network_id);

    // Get channel property
    const struct senscord_dnn_property channel_dnn = device_model_->DnnChannel();

    if (channel_dnn.network_id != ic_network_id) {
        // abnormal case
        ERR_PRINTF("device channel network_id %x is not ic_network_id %x .",
                    channel_dnn.network_id, ic_network_id);
        app_model_->ClearNextCropProperty();
        return SequenceStatus::kErrorChangeToOd;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateChangingCropIc::GetCropProperty() {
    // Get crop for used in VerifyCrop()
    struct senscord_status_t status;
    int32_t ret = senscord_->GetCrop(device_model_->CropStream(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    ret = senscord_->GetCrop(device_model_->CropChannel(), device_model_->Channel(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateChangingCropIc::VerifyCrop() {
    // Get expected crop settings
    input_tensor_dewarp_crop_property* target_crop = app_model_->NextCropProperty();
    if (target_crop == nullptr) {
        // abnormal
        return SequenceStatus::kErrorChangeToOd;
    }

    const struct input_tensor_dewarp_crop_property crop_stream = device_model_->CropStream();
    // Check crop settings
    if ((target_crop->left   != crop_stream.left)   ||
        (target_crop->top    != crop_stream.top)    ||
        (target_crop->width  != crop_stream.width)  ||
        (target_crop->height != crop_stream.height)) {
        // This means previous set crop failed, so retry set crop
        return SequenceStatus::kRetrySetProperty;
    }

    const struct input_tensor_dewarp_crop_property crop_channel = device_model_->CropChannel();
    // Check crop settings
    if ((target_crop->left   != crop_channel.left)   ||
        (target_crop->top    != crop_channel.top)    ||
        (target_crop->width  != crop_channel.width)  ||
        (target_crop->height != crop_channel.height)) {
        // crop settings are not reflected
        return SequenceStatus::kContinue;
    }

    app_model_->ClearNextCropProperty();
    return SequenceStatus::kSuccessCompleted;
}

AppState::SequenceStatus StateChangingCropIc::RetrySetCrop() {
    input_tensor_dewarp_crop_property* target_crop = app_model_->NextCropProperty();
    if (target_crop == nullptr) {
        // abnormal
        return SequenceStatus::kErrorChangeToOd;
    }

    struct senscord_status_t status;
    int32_t ret = senscord_->SetCrop(target_crop, status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}
