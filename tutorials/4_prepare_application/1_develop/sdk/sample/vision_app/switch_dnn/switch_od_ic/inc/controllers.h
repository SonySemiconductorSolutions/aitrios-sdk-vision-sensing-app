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

#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_

#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

class SenscordController;

/**
 * @class EvpController
 * @brief EVP controller class
 */
class EvpController {
private:
    pthread_mutex_t mutex_;
    pthread_cond_t  cond_;
    struct EVP_client* handle_ = nullptr;
    bool is_set_parameter      = false;

    std::function<int(const void* param)> callback_validate_param_ = nullptr;
    std::function<void(void)> callback_clear_param_ = nullptr;
    std::function<void(const void* param)> callback_set_param_ = nullptr;

public:
    /**
     * @brief Constructor
     */
    EvpController();

    /**
     * @brief Destructor
     */
    virtual ~EvpController();

    /**
     * @brief EVP initialize
     * @param [in] analyzers vector of analyzer instance
     * @return int : 0 is success, negative value is failure
     */
    int Initialize(std::function<int(const void* param)> callback_validate_param,
                    std::function<void(void)> callback_clear_param,
                    std::function<void(const void* param)> callback_set_param);

    /**
     * @brief EVP process event
     * @return EVP_RESULT
     */
    EVP_RESULT ProcessEvent();

    /**
     * @brief Wait SetParam
     */
    void WaitSetParam();

    /**
     * @brief Has Param
     */
    bool HasParam() const;

private:
    /**
     * @brief Callback function
     * @param [in] topic topic
     * @param [in] config configuration
     * @param [in] config_len size of configuration
     */
    void Callback(const char *topic, const void *config, size_t config_len);

    /**
     * @brief ConfigurationCallback
     * @param [in] topic topic
     * @param [in] config configuration
     * @param [in] config_len size of configuration
     * @param [in] private_data callback pointer
     */
    static void ConfigurationCallback(const char *topic, const void *config, size_t config_len, void *private_data);
};

/**
 * @class EvpThreadController
 * @brief EVP thread controller class
 */
class EvpThreadController {
private:
    pthread_t thread_handle_   = 0;
    EvpController* evp_        = nullptr;
    SenscordController* senscord_ = nullptr;
    bool is_thread_cancel_     = false;
    bool is_thread_terminated_ = false;
public:
    /**
     * @brief Constructor
     */
    EvpThreadController();

    /**
     * @brief Destructor
     */
    virtual ~EvpThreadController();

    /**
     * @brief Start EVP thread
     * @param [in] evp EvpController instance
     * @param [in] senscord SenscordController instance
     * @return int : 0 is success, other value is failure
     */
    int Start(EvpController* evp, SenscordController* senscord);

    /**
     * @brief Stop EVP thread
     */
    void Stop();

    /**
     * @brief Get flag if thread terminated
     * @return bool : true=thread terminated, false=thread not terminated
     */
    bool IsThreadTerminated();

private:
    /**
     * @brief Thread function
     * @param [in] arg thread argument
     */
    static void* ThreadFunc(void* arg);

    /**
     * @brief Call ProcessEvent
     */
    void Process();
};

/**
 * @class SessAllocator
 * @brief Sess allocator class
 */
class SessAllocator : public AnalyzerBase::Allocator {
public:
    /**
     * @brief Malloc
     * @param [in] size malloc size
     * @return pointer to allocated memory
     */
    void* Malloc(size_t size) const;

    /**
     * @brief Free
     * @param [in] buf buffer
     */
    void Free(void *buf) const;
};

/**
 * @class SessController
 * @brief Sess controller class
 */
class SessController {
private:
    /**
     * @class SyncSend
     * @brief Sync send class
     */
    class SyncSend {
    private:
        int send_cnt_ = 0;
        pthread_mutex_t mutex_;
        pthread_cond_t  cond_;
    public:
        /**
         * @brief Constructor
         */
        SyncSend();

        /**
         * @brief Destructor
         */
        ~SyncSend();

        /**
         * @brief Send sync
         */
        void Send();

        /**
         * @brief Send sync done
         */
        void Done();

        /**
         * @brief Wait send sync done
         */
        void WaitDone();

        /**
         * @brief Get number of sending data
         */
        int SendingCount() const;
    };
    SyncSend* sync_          = nullptr;
    bool sessinit_succeeded_ = false;
    const SessAllocator* sess_allocator_ = nullptr;
public:
    /**
     * @brief Constructor
     */
    SessController();

    /**
     * @brief Destructor
     */
    virtual ~SessController();

    /**
     * @brief Sess Initialize
     * @return int : 0 is success, negative value is failure
     */
    int Initialize(const SessAllocator* sess_allocator);

    /**
     * @brief Send data
     * @param [in] data data to send
     * @param [in] size data size
     * @param [in] timestamp timestamp
     * @return SessResult
     */
    SessResult SendData(const void *data, size_t size, uint64_t timestamp);

    /**
     * @brief Wait send data done
     */
    void WaitDataSend();

    /**
     * @brief Get number of sending data
     */
    int SendingCount() const;
private:
    /**
     * @brief Callback function
     * @param [in] buf_addr buffer pointer
     * @param [in] send_data_ret SessResult
     */
    void Callback(void *buf_addr, SessResult send_data_ret);

    /**
     * @brief Send data done callback
     * @param [in] buf_addr buffer pointer
     * @param [in] private_data callback pointer
     * @param [in] send_data_ret SessResult
     */
    static void SendDataDoneCallback(void *buf_addr, void *private_data, SessResult send_data_ret);
};

/**
 * @class SenscordController
 * @brief Senscord controller class
 */
class SenscordController {
private:
    const char* stream_key_   = SENSCORD_STREAM_KEY_IMX500_IMAGE;
    senscord_core_t core_     = nullptr;
    senscord_stream_t stream_ = nullptr;
    std::vector<senscord_frame_t> frames_;
    bool stream_stopped_ = true;
public:
    /**
     * @brief Constructor
     */
    SenscordController(const char* stream_key = SENSCORD_STREAM_KEY_IMX500_IMAGE);

    /**
     * @brief Destructor
     */
    virtual ~SenscordController();
    
    /**
     * @brief Senscord initialize
     * @return int : 0 is success, negative value is failure
     */
    int Open();

    /**
     * @brief Senscord finalize
     */
    void Close();

    /**
     * @brief Get frame
     * @param [out] frame frame
     * @param [out] status status
     * @param [in] timeout_msec timeout msec
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetFrame(senscord_frame_t& frame, struct senscord_status_t& status, int32_t timeout_msec = -1);

    /**
     * @brief Release frame
     * @param [in] frame frame
     * @return int : 0 is success, negative value is failure
     */
    int32_t ReleaseFrame(senscord_frame_t frame, struct senscord_status_t& status);

    /**
     * @brief Get channel
     * @param [in] frame frame
     * @param [in] channel_id channel id
     * @param [out] channel channel
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetChannel(const senscord_frame_t& frame, const uint32_t channel_id, senscord_channel_t& channel, struct senscord_status_t& status);

    /**
     * @brief Get raw data
     * @param [in] frame frame
     * @param [in] channel channel
     * @param [out] raw_data raw data
     * @param [in] channel_id channel id
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetRawData(const senscord_frame_t& frame, const senscord_channel_t& channel, struct senscord_raw_data_t* raw_data, struct senscord_status_t& status, uint32_t channel_id = SENSCORD_CHANNEL_ID_OUTPUT_TENSOR);

    /**
     * @brief Get channel property
     * @param [in] channel channel
     * @param [in] property_key property key
     * @param [out] value pointer to memory area of property
     * @param [in] value_size property size
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetChannelProperty(const senscord_channel_t& channel, const char* property_key, void* value, size_t value_size, struct senscord_status_t& status);

    /**
     * @brief Get stream property
     * @param [in] stream stream
     * @param [in] property_key property key
     * @param [out] value pointer to memory area of property
     * @param [in] value_size property size
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetStreamProperty(const senscord_stream_t& stream, const char* property_key, void* value, size_t value_size, struct senscord_status_t& status);

    /**
     * @brief Get crop settings
     * @param [out] crop crop settings
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetCrop(struct input_tensor_dewarp_crop_property& crop, struct senscord_status_t& status);

    /**
     * @brief Get crop settings
     * @param [out] crop crop settings
     * @param [in] channel channel
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetCrop(struct input_tensor_dewarp_crop_property& crop, const senscord_channel_t& channel, struct senscord_status_t& status);

    /**
     * @brief Set crop settings
     * @param [in] crop crop settings
     * @param [in] status senscord status
     * @return int : 0 is success, negative value is failure
     */
    int32_t SetCrop(const struct input_tensor_dewarp_crop_property* crop, struct senscord_status_t& status);

    /**
     * @brief Get dnn settings
     * @param [out] dnn dnn settings
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetDnn(struct senscord_dnn_property& dnn, struct senscord_status_t& status);

    /**
     * @brief Get dnn settings
     * @param [out] dnn dnn settings
     * @param [in] channel channel
     * @return int : 0 is success, negative value is failure
     */
    int32_t GetDnn(struct senscord_dnn_property& dnn, const senscord_channel_t& channel, struct senscord_status_t& status);

    /**
     * @brief Set dnn settings
     * @param [in] dnn dnn settings
     * @param [in] status senscord status
     * @return int : 0 is success, negative value is failure
     */
    int32_t SetDnn(const struct senscord_dnn_property* dnn, struct senscord_status_t& status);

    /**
     * @brief Stop stream
     * @return int : 0 is success, negative value is failure
     */
    int32_t StopStream();

private:
    /**
     * @brief Callback function
     * @param [in] stream
     */
    void Callback(senscord_stream_t stream);

    /**
     * @brief FrameRecvCallback
     * @param [in] stream stream
     * @param [in] private_data callback pointer
     */
    static void  FrameRecvCallback(senscord_stream_t stream, void *private_data);

    /**
     * @brief Print senscord error details
     */
    void PrintError();

    /**
     * @brief Print senscord error details
     * @return senscord_error_level_t
     */
    senscord_error_level_t GetSensCordErrorLevel();
};

#endif // _CONTROLLERS_H_
