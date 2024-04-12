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

#include "state_od.h"
#include "defines.h"

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define MIN_CROPHOFFSET (0)
#define MAX_CROPHOFFSET (4056)
#define MIN_CROPVOFFSET (0)
#define MAX_CROPVOFFSET (3040)
#define MIN_CROPHSIZE (64)
#define MAX_CROPHSIZE (4056)
#define MIN_CROPVSIZE (64)
#define MAX_CROPVSIZE (3040)

StateOd::StateOd(EvpController* evp,
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
    own_state_ = SequenceState::kOd;
    next_state_ = SequenceState::kChangingDnnIc;
}

StateOd::~StateOd() {}

AppState::SequenceStatus StateOd::VerifyDnnStream() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnStream(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t od_network_id;
    analyzer_od_->GetNetworkId(od_network_id);

    // Get stream property
    const struct senscord_dnn_property stream_dnn = device_model_->DnnStream();

    if (stream_dnn.network_id != od_network_id) {
        // first AI model's network_id doesn't exist in the device
        ERR_PRINTF("first network_id is %x but device stream network_id is %x .", od_network_id, stream_dnn.network_id);
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateOd::VerifyDnnChannel() {
    // Check whether the expected AI model is set
    struct senscord_status_t status;
    int32_t ret = senscord_->GetDnn(device_model_->DnnChannel(), device_model_->Channel(), status);
    if (ret < 0) {
        if (status.level == SENSCORD_LEVEL_FATAL) {
            return SequenceStatus::kErrorFatal;
        }
        return SequenceStatus::kContinue;
    }

    uint32_t od_network_id;
    analyzer_od_->GetNetworkId(od_network_id);

    // Get channel property
    const struct senscord_dnn_property channel_dnn = device_model_->DnnChannel();
    if (channel_dnn.network_id != od_network_id) {
        // If the expected AI model is not set, continue
        INFO_PRINTF("first network_id is %x but device channel network_id is %x .", od_network_id, channel_dnn.network_id);
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateOd::GetCropProperty() {
    // Get crop for used in SaveCrop()
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

AppState::SequenceStatus StateOd::GetRawData(struct senscord_raw_data_t& raw_data) {
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

AppState::SequenceStatus StateOd::AnalyzeData(struct senscord_raw_data_t& raw_data) {
    // Set OD timestamp
    app_model_->SetTraceId(raw_data.timestamp);
    AnalyzerBase::ResultCode analyze_ret = analyzer_od_->Analyze(static_cast<float*>(raw_data.address), raw_data.size/4, app_model_->TraceId());
    if (analyze_ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("Analyze : analyze_ret=%d", analyze_ret);
        if (analyze_ret == AnalyzerBase::ResultCode::kInvalidParam) {
            return SequenceStatus::kContinue;
        } else {
            return SequenceStatus::kError;
        }
    }

    AnalyzerOd::DetectionData data;
    analyzer_od_->GetAnalyzedData(data);
    if (data.num_of_detections_ == 0) {
        return SequenceStatus::kContinue;
    }

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateOd::SaveCrop() {
    // Save detection crop settings
    AnalyzerOd::DetectionData data;
    analyzer_od_->GetAnalyzedData(data);

    uint8_t index_for_crop = 0;
    uint8_t num_of_detections = data.num_of_detections_;
    for (uint8_t i = 0; i < num_of_detections; i++) {
        MUTEX_LOCK();
        bool used_for_cropping = data.v_is_used_for_cropping_[i];
        MUTEX_UNLOCK();
        if (used_for_cropping) {
            index_for_crop = i;
            break;
        }
    }

    // Crop has already gotten in VerifyCrop()
    const struct input_tensor_dewarp_crop_property current_crop = device_model_->CropChannel();

    uint16_t input_tensor_width;
    uint16_t input_tensor_height;
    analyzer_od_->GetInputTensorSize(input_tensor_width, input_tensor_height);

    float ratio_x = (float)current_crop.width / input_tensor_width;
    float ratio_y = (float)current_crop.height / input_tensor_height;
 
    MUTEX_LOCK();
    uint16_t xmin = data.v_bbox_[index_for_crop].m_xmin_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    uint16_t ymin = data.v_bbox_[index_for_crop].m_ymin_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    uint16_t xmax = data.v_bbox_[index_for_crop].m_xmax_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    uint16_t ymax = data.v_bbox_[index_for_crop].m_ymax_;
    MUTEX_UNLOCK();
    DBG_PRINTF(
        "data[%d] :[x_min,y_min,x_max,y_max] = [%d,%d,%d,%d]",
        index_for_crop, xmin, ymin, xmax, ymax
    );

    struct input_tensor_dewarp_crop_property* target_crop = new input_tensor_dewarp_crop_property();
    target_crop->left   = xmin * ratio_x;
    target_crop->top    = ymin * ratio_y;
    target_crop->width  = (xmax - xmin) * ratio_x;
    target_crop->height = (ymax - ymin) * ratio_y;
    // Check crop value
    if (MIN_CROPHOFFSET > target_crop->left || target_crop->left > MAX_CROPHOFFSET ||
        MIN_CROPVOFFSET > target_crop->top || target_crop->top > MAX_CROPVOFFSET ||
        MIN_CROPHSIZE > target_crop->width || target_crop->width > MAX_CROPHSIZE ||
        MIN_CROPVSIZE > target_crop->height || target_crop->height > MAX_CROPVSIZE) {
        WARN_PRINTF("Invalid crop value left:%u, top:%u, width:%u, height:%u", target_crop->left, target_crop->top, target_crop->width, target_crop->height);
        return SequenceStatus::kContinue;
    }
    app_model_->SetNextCropProperty(target_crop);

    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateOd::SerializeData(void** p_out_buf, uint32_t& out_size) {
    AnalyzerBase::ResultCode analyze_ret = analyzer_od_->Serialize(p_out_buf, &out_size, sess_allocator_);
    if (analyze_ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("Serialize : analyze_ret=%d", analyze_ret);
        return SequenceStatus::kError;
    }
    return SequenceStatus::kSuccess;
}

AppState::SequenceStatus StateOd::SendSerializedData(void* p_out_buf, const uint32_t& out_size, uint64_t timestamp) {
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

AppState::SequenceStatus StateOd::SetDnn() {
    uint32_t target_network_id;
    analyzer_ic_->GetNetworkId(target_network_id);

    struct senscord_dnn_property* dnn = new senscord_dnn_property({ target_network_id, 0 });
    app_model_->SetNextDnnProperty(dnn);

    // Set dnn to ic
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
