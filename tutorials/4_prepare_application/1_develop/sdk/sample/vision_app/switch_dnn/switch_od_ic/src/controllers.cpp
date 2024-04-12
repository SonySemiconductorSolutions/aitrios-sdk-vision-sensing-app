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

#include "controllers.h"
#include "defines.h"

namespace senscord_error_info {
    const char *s_level_str[] = {"SENSCORD_LEVEL_UNDEFINED", "SENSCORD_LEVEL_FAIL", "SENSCORD_LEVEL_FATAL"};
    const char *s_cause_str[] = {"SENSCORD_ERROR_NONE",
                                "SENSCORD_ERROR_NOT_FOUND",
                                "SENSCORD_ERROR_INVALID_ARGUMENT",
                                "SENSCORD_ERROR_RESOURCE_EXHAUSTED",
                                "SENSCORD_ERROR_PERMISSION_DENIED",
                                "SENSCORD_ERROR_BUSY",
                                "SENSCORD_ERROR_TIMEOUT",
                                "SENSCORD_ERROR_CANCELLED",
                                "SENSCORD_ERROR_ABORTED",
                                "SENSCORD_ERROR_ALREADY_EXISTS",
                                "SENSCORD_ERROR_INVALID_OPERATION",
                                "SENSCORD_ERROR_OUT_OF_RANGE",
                                "SENSCORD_ERROR_DATA_LOSS",
                                "SENSCORD_ERROR_HARDWARE_ERROR",
                                "SENSCORD_ERROR_NOT_SUPPORTED",
                                "SENSCORD_ERROR_UNKNOWN"};
}

/////////////////////////////////
// EvpController
/////////////////////////////////

EvpController::EvpController(){
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_, nullptr);
}

EvpController::~EvpController() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
}

int EvpController::Initialize(std::function<int(const void* param)> callback_validate_param,
                std::function<void(void)> callback_clear_param,
                std::function<void(const void* param)> callback_set_param) {
    MUTEX_LOCK();
    callback_validate_param_ = callback_validate_param;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    callback_clear_param_ = callback_clear_param;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    callback_set_param_ = callback_set_param;
    MUTEX_UNLOCK();

    handle_ = EVP_initialize();
    INFO_PRINTF("EVP client handle:%p", handle_);

    EVP_RESULT evp_ret = EVP_setConfigurationCallback(handle_, (EVP_CONFIGURATION_CALLBACK)ConfigurationCallback, this);
    if (EVP_OK != evp_ret) {
        ERR_PRINTF("EVP_setConfigurationCallback evp_ret:%d", evp_ret);
        return -1;
    }
    INFO_PRINTF("EVP_setConfigurationCallback evp_ret:%d", evp_ret);
    return 0;
}

EVP_RESULT EvpController::ProcessEvent() {
    static int32_t timeout_msec = 1000;
    EVP_RESULT evp_ret = EVP_processEvent(handle_, timeout_msec);
    return evp_ret;
}

void EvpController::WaitSetParam() {
    pthread_mutex_lock(&mutex_);
    if (!is_set_parameter) {
        pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
}

bool EvpController::HasParam() const {
    return is_set_parameter;
}

void EvpController::Callback(const char *topic, const void *config, size_t config_len) {
    if ((char *)config == nullptr) {
        ERR_PRINTF("Invalid param : config=nullptr");
        return;
    }

    INFO_PRINTF("%s topic:%s\nconfig:%s\nconfig_len:%zu\n", __func__, topic, (char*)config, config_len);
    
    MUTEX_LOCK();
    int str_ret = strcmp((char *)config, "");
    MUTEX_UNLOCK();
    if (str_ret == 0) {
        INFO_PRINTF("ConfigurationCallback: config is empty.");
        return;
    }
    pthread_mutex_lock(&mutex_);
    int err_cnt = 0;
    if (callback_validate_param_) {
        err_cnt += callback_validate_param_(config);
    }
    if (err_cnt != 0) {
        if (callback_clear_param_) {
            callback_clear_param_();
        }
        pthread_mutex_unlock(&mutex_);
        return;
    }

    if (callback_set_param_) {
        callback_set_param_(config);
    }

    is_set_parameter = true;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
    return;
}

void EvpController::ConfigurationCallback(const char *topic, const void *config, size_t config_len, void *private_data) {
    if (!private_data) {
        ERR_PRINTF("ConfigurationCallback : fatal private_data=nullptr");
        return;
    }
    EvpController* evp = reinterpret_cast<EvpController*>(private_data);
    evp->Callback(topic, config, config_len);
    return;
}

/////////////////////////////////
// EvpThreadController
/////////////////////////////////

EvpThreadController::EvpThreadController(){}

EvpThreadController::~EvpThreadController(){
    Stop();
}

int EvpThreadController::Start(EvpController* evp, SenscordController* senscord) {
    evp_ = evp;
    senscord_ = senscord;
    int ret = pthread_create(&thread_handle_, nullptr, ThreadFunc, this);
    if (ret != 0) {
        ERR_PRINTF("pthread_create failed for evp_Thread");
    }
    return ret;
}

void EvpThreadController::Stop() {
    if (thread_handle_) {
        is_thread_cancel_ = true;
        pthread_join(thread_handle_, nullptr);
        thread_handle_ = 0;
        is_thread_cancel_ = false;
        evp_ = nullptr;
    }
}

bool EvpThreadController::IsThreadTerminated() {
    if (is_thread_terminated_) {
        INFO_PRINTF("EVP thread terminated");
    }
    return is_thread_terminated_;
}

void* EvpThreadController::ThreadFunc(void* arg) {
    EvpThreadController* evp_thread = reinterpret_cast<EvpThreadController*>(arg);
    evp_thread->Process();
    return nullptr;
}

void EvpThreadController::Process() {
    while (!is_thread_cancel_) {
        EVP_RESULT evp_ret = evp_->ProcessEvent();
        if (evp_ret == EVP_SHOULDEXIT) {
            INFO_PRINTF("Should exit vision app");
            is_thread_terminated_ = true;
            // stop stream to cancel senscord_stream_get_frame in main thread
            int32_t ret = senscord_->StopStream();
            if (ret < 0) {
                INFO_PRINTF("StopStream : ret=%d", ret);
            }
            break;
        }
        else if (evp_ret == EVP_TIMEDOUT) {
            // Do Nothing
        }
        else if (evp_ret != EVP_OK) {
            ERR_PRINTF("Failed to EVP_processEvent:%d", evp_ret);
        }
        else {
            // Do Nothing
        }
    }
    return;
}

/////////////////////////////////
// SessAllocator
/////////////////////////////////

void* SessAllocator::Malloc(size_t size) const {
    MUTEX_LOCK();
    void* ret = SessMalloc(size);
    MUTEX_UNLOCK();
    return ret;
}

void SessAllocator::Free(void *buf) const {
    MUTEX_LOCK();
    SessFree(buf);
    MUTEX_UNLOCK();
}


/////////////////////////////////
// SessController
/////////////////////////////////

SessController::SyncSend::SyncSend() {
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_, nullptr);
}

SessController::SyncSend::~SyncSend() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
}

void SessController::SyncSend::Send() {
    pthread_mutex_lock(&mutex_);
    send_cnt_++;
    if (send_cnt_ == 0) {
        pthread_cond_signal(&cond_);
    }
    pthread_mutex_unlock(&mutex_);
}

void SessController::SyncSend::Done() {
    pthread_mutex_lock(&mutex_);
    send_cnt_--;
    if (send_cnt_ == 0) {
        pthread_cond_signal(&cond_);
    }
    pthread_mutex_unlock(&mutex_);
}

void SessController::SyncSend::WaitDone() {
    pthread_mutex_lock(&mutex_);
    if (send_cnt_ != 0) {
        pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
}

int SessController::SyncSend::SendingCount() const {
    return send_cnt_;
}

SessController::SessController(){}

SessController::~SessController(){
    SessResult sess_ret = kSessOther;
    if (sessinit_succeeded_) {
        sess_ret = SessUnregisterSendDataCallback();
        if (sess_ret != kSessOK) {
            ERR_PRINTF("SessUnregisterSendDataCallback : sess_ret=%d", sess_ret);
        }
    }
    sess_ret = SessExit();
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessExit : sess_ret=%d", sess_ret);
    }
    if (sync_) {
        delete sync_;
    }
}

int SessController::Initialize(const SessAllocator* sess_allocator) {
    sess_allocator_ = sess_allocator;
    SessResult sess_ret = SessInit();
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessInit : sess_ret=%d", sess_ret);
        return -1;
    }
    sessinit_succeeded_ = true;
    sess_ret = SessRegisterSendDataCallback(SendDataDoneCallback, this);
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessRegisterSendDataCallback : sess_ret=%d", sess_ret);
        return -1;
    }
    sync_ = new SyncSend();
    return 0;
}

SessResult SessController::SendData(const void *data, size_t size, uint64_t timestamp) {
    if (!sessinit_succeeded_) {
        ERR_PRINTF("SendData : SessInit has not succeeded");
        return kSessOther;
    }
    SessResult sess_ret = SessSendData(data, size, timestamp);
    if (sess_ret == kSessOK) {
        sync_->Send();
    }
    else if (sess_ret == kSessNotStreaming) {
        DBG_PRINTF("camera not streaming : sess_ret=%d", sess_ret);
    }
    else {
        ERR_PRINTF("SessSendData : sess_ret=%d", sess_ret);
    }
    return sess_ret;
}

void SessController::WaitDataSend() {
    sync_->WaitDone();
}

int SessController::SendingCount() const {
    return sync_->SendingCount();
}

void SessController::Callback(void *buf_addr, SessResult send_data_ret){
    if (buf_addr != nullptr) {
        sess_allocator_->Free(buf_addr);
    }
    if (send_data_ret != kSessOK) {
        ERR_PRINTF("SendDataDoneCallback : send_data_ret:%d", send_data_ret);
    }
    sync_->Done();
}

void SessController::SendDataDoneCallback(void *buf_addr, void *private_data, SessResult send_data_ret){
    if (!private_data) {
        ERR_PRINTF("SendDataDoneCallback : fatal private_data=nullptr");
        return;
    }
    SessController* sess = reinterpret_cast<SessController*>(private_data);
    sess->Callback(buf_addr, send_data_ret);
}

/////////////////////////////////
// SenscordController
/////////////////////////////////

SenscordController::SenscordController(const char* stream_key) : stream_key_(stream_key){}

SenscordController::~SenscordController() {
    Close();
}

int SenscordController::Open() {
    int32_t ret = senscord_core_init(&core_);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_init : ret=%d", ret);
        PrintError();
        return -1;
    }
    ret = senscord_core_open_stream(core_, stream_key_, &stream_);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_open_stream : ret=%d", ret);
        PrintError();
        return -1;
    }
    ret = senscord_stream_start(stream_);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_start : ret=%d", ret);
        PrintError();
        return -1;
    }
    stream_stopped_ = false;
    ret = senscord_stream_register_frame_callback(stream_, FrameRecvCallback, this);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_register_frame_callback : ret=%d", ret);
        PrintError();
        return -1;
    }
    return 0;
}

void SenscordController::Close() {
    if (!stream_) {
        return;
    }
    int32_t ret = -1;
    MUTEX_LOCK();
    std::size_t num_of_frame = frames_.size();
    MUTEX_UNLOCK();
    for (std::size_t i = 0; i < num_of_frame; i++) {
        MUTEX_LOCK();
        auto& frame = frames_[i];
        MUTEX_UNLOCK();
        ret = senscord_stream_release_frame(stream_, frame);
        if (ret < 0) {
            ERR_PRINTF("senscord_stream_release_frame : ret=%d", ret);
            PrintError();
        }
    }
    MUTEX_LOCK();
    frames_.clear();
    MUTEX_UNLOCK();
    ret = senscord_stream_unregister_frame_callback(stream_);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_unregister_frame_callback : ret=%d", ret);
        PrintError();
    }
    if (!stream_stopped_) {
        ret = senscord_stream_stop(stream_);
        if (ret < 0) {
            ERR_PRINTF("senscord_stream_stop : ret=%d", ret);
            PrintError();
        } else {
            stream_stopped_ = true;
        }
    }
    ret = senscord_core_close_stream(core_,stream_);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_close_stream : ret=%d", ret);
        PrintError();
    }
    stream_ = nullptr;
    ret = senscord_core_exit(core_);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_exit : ret=%d", ret);
        PrintError();
    }
    core_ = nullptr;
}

int32_t SenscordController::GetFrame(senscord_frame_t& frame, struct senscord_status_t& status, int32_t timeout_msec) {
    if (!stream_) {
        ERR_PRINTF("GetFrame : stream_=nullptr");
        return -1;
    }
    int32_t ret = senscord_stream_get_frame(stream_, &frame, timeout_msec);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_get_frame : ret=%d", ret);
        senscord_get_last_error(&status);
        if (status.cause == SENSCORD_ERROR_TIMEOUT) {
            INFO_PRINTF("senscord_stream_get_frame TIMEOUT");
        }
        else {
            ERR_PRINTF("senscord_stream_get_frame : ret=%d", ret);
            PrintError();
        }
        return ret;
    }
    MUTEX_LOCK();
    frames_.push_back(frame);
    MUTEX_UNLOCK();
    return 0;
}

int32_t SenscordController::ReleaseFrame(senscord_frame_t frame, struct senscord_status_t& status) {
    if (!stream_) {
        ERR_PRINTF("ReleaseFrame : stream_=nullptr");
        return -1;
    }
    int32_t ret = senscord_stream_release_frame(stream_, frame);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_release_frame : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
        return ret;
    }
    MUTEX_LOCK();
    for (auto it = frames_.begin(); it != frames_.end();) {
        if (*it == frame) {
            it = frames_.erase(it);
        }
        else {
            ++it;
        }
    }
    MUTEX_UNLOCK();
    return 0;
}

int32_t SenscordController::GetChannel(const senscord_frame_t& frame, const uint32_t channel_id, senscord_channel_t& channel, struct senscord_status_t& status) {
    int32_t ret = senscord_frame_get_channel_from_channel_id(frame, channel_id, &channel);
    if (ret < 0) {
        ERR_PRINTF("senscord_frame_get_channel_from_channel_id : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    return ret;
}

int32_t SenscordController::GetRawData(const senscord_frame_t& frame, const senscord_channel_t& channel, struct senscord_raw_data_t* raw_data, struct senscord_status_t& status, uint32_t channel_id){
    int32_t ret = senscord_channel_get_raw_data(channel, raw_data);
    if (ret < 0) {
        ERR_PRINTF("senscord_channel_get_raw_data : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    return ret;
}

int32_t SenscordController::GetChannelProperty(const senscord_channel_t& channel, const char* property_key, void* value, size_t value_size, struct senscord_status_t& status) {
    int32_t ret = senscord_channel_get_property(channel, property_key, value, value_size);
    if (ret < 0) {
        ERR_PRINTF("senscord_channel_get_property : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    return ret;
}

int32_t SenscordController::GetStreamProperty(const senscord_stream_t& stream, const char* property_key, void* value, size_t value_size, struct senscord_status_t& status) {
    if (!stream_) {
        ERR_PRINTF("GetStreamProperty : stream_=nullptr");
        return -1;
    }
    int32_t ret = senscord_stream_get_property(stream, property_key, value, value_size);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_get_property : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    return ret;
}

int32_t SenscordController::GetCrop(struct input_tensor_dewarp_crop_property& crop, struct senscord_status_t& status) {
    int32_t ret = -1;
    ret = GetStreamProperty(stream_, SENSCORD_PROPERTY_KEY_DEWARP_CROP, &crop, sizeof(struct input_tensor_dewarp_crop_property), status);
    if (!ret) {
        DBG_PRINTF("GetCrop stream : left=%d, top=%d, width=%d, height=%d", crop.left, crop.top, crop.width, crop.height);
    }
    if (ret != 0) {
        ERR_PRINTF("GetCrop stream : ret=%d", ret);
    }
    return ret;
}

int32_t SenscordController::GetCrop(struct input_tensor_dewarp_crop_property& crop, const senscord_channel_t& channel, struct senscord_status_t& status) {
    int32_t ret = -1;
    ret = GetChannelProperty(channel, SENSCORD_PROPERTY_KEY_DEWARP_CROP, &crop, sizeof(struct input_tensor_dewarp_crop_property), status);
    if (!ret) {
        DBG_PRINTF("GetCrop channel : left=%d, top=%d, width=%d, height=%d", crop.left, crop.top, crop.width, crop.height);
    }
    if (ret != 0) {
        ERR_PRINTF("GetCrop channel : ret=%d", ret);
    }
    return ret;
}

int32_t SenscordController::SetCrop(const struct input_tensor_dewarp_crop_property* crop, struct senscord_status_t& status) {
    if (!stream_) {
        ERR_PRINTF("SetCrop : stream_=nullptr");
        return -1;
    }
    int32_t ret = senscord_stream_set_property(stream_, SENSCORD_PROPERTY_KEY_DEWARP_CROP, crop, sizeof(struct input_tensor_dewarp_crop_property));
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_set_property crop : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    else {
        DBG_PRINTF("SetCrop stream : left=%d, top=%d, width=%d, height=%d", crop->left, crop->top, crop->width, crop->height);
    }
    return ret;
}

int32_t SenscordController::GetDnn(struct senscord_dnn_property& dnn, struct senscord_status_t& status) {
    int32_t ret = -1;
    ret = GetStreamProperty(stream_, SENSCORD_PROPERTY_KEY_DNN, &dnn, sizeof(struct senscord_dnn_property), status);
    if (!ret) {
        INFO_PRINTF("GetDnn stream : Id=0x%x, Ord=%d", dnn.network_id, dnn.network_ordinal);
    }
    if (ret != 0) {
        ERR_PRINTF("GetDnn stream : ret=%d", ret);
    }
    return ret;
}

int32_t SenscordController::GetDnn(struct senscord_dnn_property& dnn, const senscord_channel_t& channel, struct senscord_status_t& status) {
    int32_t ret = -1;
    ret = GetChannelProperty(channel, SENSCORD_PROPERTY_KEY_DNN, &dnn, sizeof(struct senscord_dnn_property), status);
    if (!ret) {
        INFO_PRINTF("GetDnn channel : Id=0x%x, Ord=%d", dnn.network_id, dnn.network_ordinal);
    }
    if (ret != 0) {
        ERR_PRINTF("GetDnn channel : ret=%d", ret);
    }
    return ret;
}

int32_t SenscordController::SetDnn(const struct senscord_dnn_property* dnn, struct senscord_status_t& status) {
    if (!stream_) {
        ERR_PRINTF("SetDnn : stream_=nullptr");
        return -1;
    }
    int32_t ret = senscord_stream_set_property(stream_, SENSCORD_PROPERTY_KEY_DNN, dnn, sizeof(struct senscord_dnn_property));
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_set_property dnn : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError();
    }
    else {
        INFO_PRINTF("SetDnn stream : Id=0x%x, Ord=%d", dnn->network_id, dnn->network_ordinal);
    }
    return ret;
}

int32_t SenscordController::StopStream() {
    int32_t ret = 0;
    if (stream_ != nullptr && !stream_stopped_) {
        ret = senscord_stream_stop(stream_);
        if (ret < 0) {
            ERR_PRINTF("StopStream senscord_stream_stop : ret=%d", ret);
            PrintError();
        } else {
            stream_stopped_ = true;
        }
    }
    return ret;
}

void SenscordController::Callback(senscord_stream_t stream) {
    return;
}

void SenscordController::FrameRecvCallback(senscord_stream_t stream, void *private_data) {
    if (!private_data) {
        ERR_PRINTF("FrameRecvCallback : fatal private_data=nullptr");
        return;
    }
    INFO_PRINTF("Frame recv. stream:%p, private_data:%p", stream, private_data);
    SenscordController* senscord = reinterpret_cast<SenscordController*>(private_data);
    senscord->Callback(stream);
    return;
}

void SenscordController::PrintError() {
    struct senscord_status_t status;
    int32_t ret = senscord_get_last_error(&status);
    if (!ret) {
        ERR_PRINTF("status:\n - level  : %s\n - cause  : %s\n - message: %s\n - block  : %s",
            senscord_error_info::s_level_str[status.level], senscord_error_info::s_cause_str[status.cause], status.message, status.block);
    } else {
        ERR_PRINTF("senscord_get_last_error : ret=%d", ret);
    }
}

senscord_error_level_t SenscordController::GetSensCordErrorLevel() {
    struct senscord_status_t status;
    int32_t ret = senscord_get_last_error(&status);
    if (!ret) {
        ERR_PRINTF("status:\n - level  : %s\n - cause  : %s\n - message: %s\n - block  : %s",
            senscord_error_info::s_level_str[status.level], senscord_error_info::s_cause_str[status.cause], status.message, status.block);
        
        if (status.level == SENSCORD_LEVEL_UNDEFINED) {
            return SENSCORD_LEVEL_UNDEFINED;
        } else if (status.level == SENSCORD_LEVEL_FAIL) {
            return SENSCORD_LEVEL_FAIL;
        } else {
            return SENSCORD_LEVEL_FATAL;
        }
    } else {
        ERR_PRINTF("senscord_get_last_error : ret=%d", ret);
        return SENSCORD_LEVEL_FATAL;
    }
}
