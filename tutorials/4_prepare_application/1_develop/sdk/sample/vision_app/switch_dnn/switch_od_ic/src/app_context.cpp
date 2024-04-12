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

#include "app_context.h"
#include "defines.h"

#include "state_od.h"
#include "state_changing_dnn_ic.h"
#include "state_changing_crop_ic.h"
#include "state_ic.h"
#include "state_changing_dnn_od.h"

AppContext::AppContext(EvpController* evp,
    EvpThreadController* evp_thread,
    SessController* sess,
    SessAllocator* sess_allocator,
    SenscordController* senscord,
    AnalyzerCommon* analyzer_com,
    AnalyzerOd* analyzer_od,
    AnalyzerIc* analyzer_ic,
    DeviceModel* device_model,
    AppModel* app_model) : state_(nullptr) {
    impl_states_[static_cast<int>(SequenceState::kOd)] = new StateOd(evp, evp_thread, sess, sess_allocator, senscord,
                                                                    analyzer_com, analyzer_od, analyzer_ic,
                                                                    device_model, app_model);

    impl_states_[static_cast<int>(SequenceState::kChangingDnnIc)] = new StateChangingDnnIc(evp, evp_thread, sess, sess_allocator, senscord,
                                                                    analyzer_com, analyzer_od, analyzer_ic,
                                                                    device_model, app_model);

    impl_states_[static_cast<int>(SequenceState::kChangingCropIc)] = new StateChangingCropIc(evp, evp_thread, sess, sess_allocator, senscord,
                                                                    analyzer_com, analyzer_od, analyzer_ic,
                                                                    device_model, app_model);

    impl_states_[static_cast<int>(SequenceState::kIc)] = new StateIc(evp, evp_thread, sess, sess_allocator, senscord,
                                                                    analyzer_com, analyzer_od, analyzer_ic,
                                                                    device_model, app_model);

    impl_states_[static_cast<int>(SequenceState::kChangingDnnOd)] = new StateChangingDnnOd(evp, evp_thread, sess, sess_allocator, senscord,
                                                                    analyzer_com, analyzer_od, analyzer_ic,
                                                                    device_model, app_model);

    ChangeState(SequenceState::kOd);
}

AppContext::~AppContext() {
}

void AppContext::ChangeState(SequenceState state) {
    if (state_ != nullptr) {
        VER_PRINTF("[SEQ] ChangeState from %d to %d", state_->OwnState(), state);
    } else {
        VER_PRINTF("[SEQ] ChangeState to %d", state);
    }
    state_ = impl_states_[static_cast<int>(state)];
}

AppState* AppContext::State() const {
    return state_;
}

AppState::SequenceStatus AppContext::GetFrame() {
    return state_->GetFrame();
}

AppState::SequenceStatus AppContext::VerifyDnnStream() {
    return state_->VerifyDnnStream();
}

AppState::SequenceStatus AppContext::RetrySetDnn() {
    return state_->RetrySetDnn();
}

AppState::SequenceStatus AppContext::GetChannel() {
    return state_->GetChannel();
}

AppState::SequenceStatus AppContext::VerifyDnnChannel() {
    return state_->VerifyDnnChannel();
}

AppState::SequenceStatus AppContext::GetCropProperty() {
    return state_->GetCropProperty();
}

AppState::SequenceStatus AppContext::SetCrop() {
    return state_->SetCrop();
}

AppState::SequenceStatus AppContext::VerifyCrop() {
    return state_->VerifyCrop();
}

AppState::SequenceStatus AppContext::RetrySetCrop() {
    return state_->RetrySetCrop();
}

AppState::SequenceStatus AppContext::GetRawData(struct senscord_raw_data_t& raw_data) {
    return state_->GetRawData(raw_data);
}

AppState::SequenceStatus AppContext::AnalyzeData(struct senscord_raw_data_t& raw_data) {
    return state_->AnalyzeData(raw_data);
}

AppState::SequenceStatus AppContext::SaveCrop() {
    return state_->SaveCrop();
}

AppState::SequenceStatus AppContext::SerializeData(void** p_out_buf, uint32_t& out_size) {
    return state_->SerializeData(p_out_buf, out_size);
}

AppState::SequenceStatus AppContext::SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) {
    return state_->SendSerializedData(p_out_buf, out_size, timestamp);
}

AppState::SequenceStatus AppContext::SetDnn() {
    return state_->SetDnn();
}

void AppContext::ReleaseFrame() {
    state_->ReleaseFrame();
}

SequenceState AppContext::NextState() {
    return state_->NextState();
}
