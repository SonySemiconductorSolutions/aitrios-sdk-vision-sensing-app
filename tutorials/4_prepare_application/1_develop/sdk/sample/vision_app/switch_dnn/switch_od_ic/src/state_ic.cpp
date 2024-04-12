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

#include "state_ic.h"
#include "defines.h"

StateIc::StateIc(EvpController* evp,
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
    own_state_ = SequenceState::kIc;
    next_state_ = SequenceState::kChangingDnnOd;
}

StateIc::~StateIc() {}

AppState::SequenceStatus StateIc::VerifyDnnStream() {
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
            return SequenceStatus::kErrorChangeToOd;
        }

        // This may occurred reset by stop, start inference
        return SequenceStatus::kErrorChangeToOd;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateIc::VerifyDnnChannel() {
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

    uint32_t od_network_id;
    analyzer_od_->GetNetworkId(od_network_id);

    // Get channel property
    const struct senscord_dnn_property channel_dnn = device_model_->DnnChannel();

    if (channel_dnn.network_id != ic_network_id) {
        // abnormal case
        ERR_PRINTF("device channel network_id %x is not ic_network_id %x .",
                    channel_dnn.network_id, ic_network_id);
        return SequenceStatus::kErrorChangeToOd;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateIc::GetRawData(struct senscord_raw_data_t& raw_data) {
    struct senscord_status_t status;
    int32_t ret = senscord_->GetRawData(device_model_->Frame(), device_model_->Channel(), &raw_data, status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }
    INFO_PRINTF("raw_data.address:%p", raw_data.address);
    INFO_PRINTF("raw_data.size:%zu", raw_data.size);
    INFO_PRINTF("raw_data.timestamp:%llu", raw_data.timestamp);
    INFO_PRINTF("raw_data.type:%s", raw_data.type);

    return SequenceStatus::kSuccess;
};

AppState::SequenceStatus StateIc::AnalyzeData(struct senscord_raw_data_t& raw_data) {
    AnalyzerBase::ResultCode analyze_ret = analyzer_ic_->Analyze(static_cast<float*>(raw_data.address), raw_data.size/4, app_model_->TraceId());
    if (analyze_ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("Analyze : analyze_ret=%d", analyze_ret);
        return SequenceStatus::kContinue;
    }
    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateIc::SerializeData(void** p_out_buf, uint32_t& out_size) {
    AnalyzerBase::ResultCode analyze_ret = analyzer_ic_->Serialize(p_out_buf, &out_size, sess_allocator_);
    if (analyze_ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("Serialize : analyze_ret=%d", analyze_ret);
        return SequenceStatus::kError;
    }
    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateIc::SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) {
    if (SESS_SEND_MAX_SIZE < out_size) {
        DBG_PRINTF("SessSendData data size is over : size=%u", out_size);
        sess_allocator_->Free(p_out_buf);
        return SequenceStatus::kContinue;
    }

    SessResult sess_ret = sess_->SendData(p_out_buf, out_size, timestamp);
    if (sess_ret == kSessOK) {
        // sess_allocator_->Free will be called when SendDataDoneCallback called in SessController
        return SequenceStatus::kSuccess;
    } else if (sess_ret == kSessNotStreaming) {
        DBG_PRINTF("SendData camera not streaming : sess_ret=%d", sess_ret);
        sess_allocator_->Free(p_out_buf);
        return SequenceStatus::kContinue;
    }

    sess_allocator_->Free(p_out_buf);
    return SequenceStatus::kError;
}

AppState::SequenceStatus StateIc::SetDnn() {
    uint32_t target_network_id;
    analyzer_od_->GetNetworkId(target_network_id);

    struct senscord_dnn_property* dnn = new senscord_dnn_property({ target_network_id, 0 });
    app_model_->SetNextDnnProperty(dnn);

    // Set dnn to od
    struct senscord_status_t status;
    int32_t ret = senscord_->SetDnn(dnn, status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        } else {
            // Do nothing. Retry SetDNN in next state
        }
    }

    return SequenceStatus::kSuccessCompleted;
}
