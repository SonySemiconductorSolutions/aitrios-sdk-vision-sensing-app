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

#include "state_base.h"
#include "defines.h"

StateBase::StateBase(EvpController* evp,
    EvpThreadController* evp_thread,
    SessController* sess,
    SessAllocator* sess_allocator,
    SenscordController* senscord,
    AnalyzerCommon* analyzer_com,
    DeviceModel* device_model,
    AppModel* app_model) : AppState(),
        evp_(evp),
        evp_thread_(evp_thread),
        sess_(sess),
        sess_allocator_(sess_allocator),
        senscord_(senscord),
        analyzer_com_(analyzer_com),
        device_model_(device_model),
        app_model_(app_model),
        own_state_(SequenceState::kOd),
        next_state_(SequenceState::kOd) {
}

StateBase::~StateBase() {}

SequenceState StateBase::OwnState() const {
    return own_state_;
}

AppState::SequenceStatus StateBase::VerifyDnnStream() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::RetrySetDnn() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::GetFrame() {
    // Get frame
    struct senscord_status_t status;
    int32_t ret = senscord_->GetFrame(device_model_->Frame(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateBase::GetChannel() {
    // Get channel
    struct senscord_status_t status;
    int32_t ret = senscord_->GetChannel(device_model_->Frame(), SENSCORD_CHANNEL_ID_OUTPUT_TENSOR, device_model_->Channel(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateBase::VerifyDnnChannel() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::GetCropProperty() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::SetCrop() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::VerifyCrop() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::RetrySetCrop() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::GetRawData(struct senscord_raw_data_t& raw_data) {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::AnalyzeData(struct senscord_raw_data_t& raw_data) {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::SaveCrop() {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::SerializeData(void** p_out_buf, uint32_t& out_size) {
    return SequenceStatus::kNop;
}

AppState::SequenceStatus StateBase::SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) {
    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateBase::SetDnn() {
    return SequenceStatus::kNop;
}

void StateBase::ReleaseFrame() {
    struct senscord_status_t status;
    senscord_->ReleaseFrame(device_model_->Frame(), status);
}

SequenceState StateBase::NextState() {
    return next_state_;
}

void StateBase::SetNextState(SequenceState state) {
    next_state_ = state;
}
