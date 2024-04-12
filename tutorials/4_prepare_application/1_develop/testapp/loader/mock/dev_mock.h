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
#include <string>
#include <list>
#include <iterator>
#include <mutex>
#include "vision_app_public.h"

//Util
namespace MockUtil
{
    int Output_BinaryFile(void *, uint32_t, char*);
    void mock_default_sleep();
    void mock_default_sleep_short();
    std::string read_file(const std::string&);
    std::string get_loader_path();
}

//Mock Sequence Control
class mock_sequence_controller {
private:
    mock_sequence_controller();
    virtual ~mock_sequence_controller() {}

    // Wasm force stop
    bool Wasm_Force_Stop_flg_;
    // ConfigurationCallback confirmed processed
    int ConfigurationCallback_done_flg_;

public:
    static mock_sequence_controller& GetInstance();
    bool IsNeedForceStop();
    void SetForceStopNeed(bool);
    bool IsDoneConfigurationCallback();
    void SetConfigurationCallbackDone(bool);
};

// Callback function interface
class Callback_if {
public:
    Callback_if();
    virtual ~Callback_if() {}

    wasm_exec_env_t exec_env_;
    void* instance_;
};

//EVP
class EVP_mock {
private:
    EVP_mock();
    virtual ~EVP_mock() {}

    static void *call_ConfigurationCallback(void *arg);
    EVP_CONFIGURATION_CALLBACK ConfigurationCallbackFunction_;
    void* ConfigurationPrivateData_;
    int Count_EVP_processEvent_;

public:
    static EVP_mock& GetInstance();
    struct EVP_client* EVP_initialize(wasm_exec_env_t exec_env);
    EVP_RESULT EVP_setConfigurationCallback(wasm_exec_env_t exec_env, struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK cb, void* private_data);
    EVP_RESULT EVP_processEvent(wasm_exec_env_t exec_env, struct EVP_client* handle, int timeout_ms);
};


//Senscord
class Senscord_mock {
private:
    Senscord_mock();
    virtual ~Senscord_mock() {}

    void call_FrameRecvCallback(wasm_exec_env_t);
    senscord_frame_received_callback FrameRecvCallbackFunction_;
    void* private_data_;
    senscord_stream_t stream_;
    int SensCord_error_cause_;
    struct input_tensor_dewarp_crop_property crop_property_;
    struct input_tensor_dewarp_crop_property crop_default_;
    struct senscord_dnn_property dnn_property_;

    class channel_mock {
    public:
        channel_mock();
        virtual ~channel_mock(){}
        int id_;
        struct senscord_raw_data_t* raw_data_;  // wasm addr.
    };
    class frame_mock {
    public:
        frame_mock();
        virtual ~frame_mock(){}
        int channel_cnt_;
        channel_mock* channel_;
    };
    class stream_mock {
    public:
        stream_mock();
        virtual ~stream_mock(){}
        std::list<frame_mock*> frame_list_;
        std::list<frame_mock*>::iterator itr_;
    };

public:
    static Senscord_mock& GetInstance();
    int32_t senscord_core_init(wasm_exec_env_t exec_env, senscord_core_t* core);
    int32_t senscord_core_exit(wasm_exec_env_t exec_env, senscord_core_t core);
    int32_t senscord_core_open_stream(wasm_exec_env_t exec_env, senscord_core_t core, const char* stream_key, senscord_stream_t* stream);
    int32_t senscord_core_close_stream(wasm_exec_env_t exec_env, senscord_core_t core, senscord_stream_t stream);
    int32_t senscord_stream_start(wasm_exec_env_t exec_env, senscord_stream_t stream);
    int32_t senscord_stream_stop(wasm_exec_env_t exec_env, senscord_stream_t stream); 
    int32_t senscord_stream_get_frame(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t* frame, int32_t timeout_msec);
    int32_t senscord_stream_release_frame(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t frame);
    int32_t senscord_stream_get_property(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, void* value, size_t value_size);
    int32_t senscord_stream_set_property(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, const void* value, size_t value_size);
    int32_t senscord_stream_register_frame_callback(wasm_exec_env_t exec_env, senscord_stream_t stream, const senscord_frame_received_callback callback, void* private_data);
    int32_t senscord_stream_unregister_frame_callback(wasm_exec_env_t exec_env, senscord_stream_t stream);
    int32_t senscord_frame_get_channel_from_channel_id(wasm_exec_env_t exec_env, senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel);
    int32_t senscord_channel_get_raw_data(wasm_exec_env_t exec_env, senscord_channel_t channel, struct senscord_raw_data_t* raw_data);
    int32_t senscord_channel_get_property(wasm_exec_env_t exec_env, senscord_channel_t channel, const char* property_key, void* value, size_t value_size);
    int32_t senscord_get_last_error(wasm_exec_env_t exec_env, struct senscord_status_t* status);
};

//Sess
class Sess_mock {
private:
    Sess_mock();
    virtual ~Sess_mock() {}

    static void *call_SendDataDoneCallback(void *arg);
    SessSendDataCallback callback_;
    void* private_data_;
    void* data_;
    int output_cnt_;
    
public:
    static Sess_mock& GetInstance();
    SessResult SessInit(wasm_exec_env_t exec_env);
    SessResult SessExit(wasm_exec_env_t exec_env);
    SessResult SessSendData(wasm_exec_env_t exec_env, const void *data, size_t size, uint64_t timestamp); // timestamp: nanoseconds
    SessResult SessRegisterSendDataCallback(wasm_exec_env_t exec_env, SessSendDataCallback cb, void *private_data);
    SessResult SessUnregisterSendDataCallback(wasm_exec_env_t exec_env);
};

//TESTAPP_memory_dump
class TESTAPP_Memory_dump {
public:
    TESTAPP_Memory_dump();
    virtual ~TESTAPP_Memory_dump() {};

    static TESTAPP_Memory_dump& GetInstance();
    void TESTAPP_dump_mem_consumption(wasm_exec_env_t exec_env);
};
