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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parson.h"
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "wasm_export.h"
#include "wasm_runtime_common.h"

#include "vision_app_public.h"

#include "dev_mock.h"

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define ERR_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define WARN_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define INFO_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define DBG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define VER_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")

Senscord_mock::stream_mock::stream_mock() :
    itr_(NULL)
{
}
Senscord_mock::frame_mock::frame_mock() :
    channel_cnt_(1)
    ,channel_(NULL)
{
}
Senscord_mock::channel_mock::channel_mock() :
    id_(1)
    ,raw_data_(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Wasm IF
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
struct EVP_client* EVP_initialize_wrapper(wasm_exec_env_t exec_env)
{
    return EVP_mock::GetInstance().EVP_initialize(exec_env);
}

EVP_RESULT EVP_setConfigurationCallback_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK  cb, void* private_data)
{
    return EVP_mock::GetInstance().EVP_setConfigurationCallback(exec_env, handle, cb, private_data);
}

EVP_RESULT EVP_processEvent_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, int timeout_ms)
{
    return EVP_mock::GetInstance().EVP_processEvent(exec_env, handle, timeout_ms);
}

int32_t senscord_core_init_wrapper(wasm_exec_env_t exec_env, senscord_core_t* core)
{
    return Senscord_mock::GetInstance().senscord_core_init(exec_env, core);
}

int32_t senscord_core_exit_wrapper(wasm_exec_env_t exec_env, senscord_core_t core)
{
    return Senscord_mock::GetInstance().senscord_core_exit(exec_env, core);
}

int32_t senscord_core_open_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, const char* stream_key, senscord_stream_t* stream)
{
    return Senscord_mock::GetInstance().senscord_core_open_stream(exec_env, core, stream_key, stream);
}

int32_t senscord_core_close_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, senscord_stream_t stream)
{
    return Senscord_mock::GetInstance().senscord_core_close_stream(exec_env, core, stream);
}

int32_t senscord_stream_start_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    return Senscord_mock::GetInstance().senscord_stream_start(exec_env, stream);
}

int32_t senscord_stream_stop_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    return Senscord_mock::GetInstance().senscord_stream_stop(exec_env, stream);
}

int32_t senscord_stream_get_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream_app, senscord_frame_t* frame_app, int32_t timeout_msec)
{
    return Senscord_mock::GetInstance().senscord_stream_get_frame(exec_env, stream_app, frame_app, timeout_msec);
}

int32_t senscord_stream_release_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t frame)
{
    return Senscord_mock::GetInstance().senscord_stream_release_frame(exec_env, stream, frame);
}

int32_t senscord_stream_get_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, void* value, size_t value_size)
{
    return Senscord_mock::GetInstance().senscord_stream_get_property(exec_env, stream, property_key, value, value_size);
}

int32_t senscord_stream_set_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, const void* value, size_t value_size)
{
    return Senscord_mock::GetInstance().senscord_stream_set_property(exec_env, stream, property_key, value, value_size);
}

int32_t senscord_stream_register_frame_callback_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_received_callback callback, void* private_data)
{
    return Senscord_mock::GetInstance().senscord_stream_register_frame_callback(exec_env, stream, callback, private_data);
}

int32_t senscord_stream_unregister_frame_callback_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    return Senscord_mock::GetInstance().senscord_stream_unregister_frame_callback(exec_env, stream);
}

int32_t senscord_frame_get_channel_from_channel_id_wrapper(wasm_exec_env_t exec_env, senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel)
{
    return Senscord_mock::GetInstance().senscord_frame_get_channel_from_channel_id(exec_env, frame, channel_id, channel);
}

int32_t senscord_channel_get_raw_data_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, struct senscord_raw_data_t* raw_data_app)
{
    return Senscord_mock::GetInstance().senscord_channel_get_raw_data(exec_env, channel_app, raw_data_app);
}

int32_t senscord_channel_get_property_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, const char* property_key_app, void* value_app, size_t value_size)
{
    return Senscord_mock::GetInstance().senscord_channel_get_property(exec_env, channel_app, property_key_app, value_app, value_size);
}

int32_t senscord_get_last_error_wrapper(wasm_exec_env_t exec_env, senscord_status_t* status)
{
    return Senscord_mock::GetInstance().senscord_get_last_error(exec_env, status);
}

SessResult SessInit_wrapper(wasm_exec_env_t exec_env)
{
    return Sess_mock::GetInstance().SessInit(exec_env);
}

SessResult SessExit_wrapper(wasm_exec_env_t exec_env)
{
    return Sess_mock::GetInstance().SessExit(exec_env);
}

SessResult SessSendData_wrapper(wasm_exec_env_t exec_env, const void *data, size_t size, uint64_t timestamp)
{
    return Sess_mock::GetInstance().SessSendData(exec_env, data, size, timestamp);
}

SessResult SessRegisterSendDataCallback_wrapper(wasm_exec_env_t exec_env, SessSendDataCallback cb, void *private_data)
{
    return Sess_mock::GetInstance().SessRegisterSendDataCallback(exec_env, cb, private_data);
}

SessResult SessUnregisterSendDataCallback_wrapper(wasm_exec_env_t exec_env)
{
    return Sess_mock::GetInstance().SessUnregisterSendDataCallback(exec_env);
}

void TESTAPP_dump_mem_consumption_wrapper(wasm_exec_env_t exec_env)
{
    TESTAPP_Memory_dump::GetInstance().TESTAPP_dump_mem_consumption(exec_env);
}

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
// Implement

//////////////////////////////////////////////////////////////////////////////////////////////////
// Util
//////////////////////////////////////////////////////////////////////////////////////////////////
namespace MockUtil
{
    std::string get_loader_path()
    {
        const size_t LINKSIZE = 100;
        char link[LINKSIZE];

        char *loader_path = (char *)malloc(sizeof(char) * PATH_MAX);
        memset(loader_path, '\0', sizeof(char) * PATH_MAX);
        snprintf(link, LINKSIZE, "/proc/%d/exe", getpid() );
        readlink(link, loader_path, sizeof(char) * PATH_MAX);
        char* pos = strrchr(loader_path, '/');
        *pos = '\0';
        std::string return_val(loader_path); 
        free(loader_path);

        return (return_val);
    }

    int Output_BinaryFile(void *p_out_buf, uint32_t out_size, const char* p_filename)
    {
        FILE* fp;
    
        INFO_PRINTF("mock (internal function) Output_BinaryFile datasize:%u", out_size);
    
        fp = fopen(p_filename, "wb");
        if (fp == NULL) {
            ERR_PRINTF("Output_BinaryFile file open error errno: %d", errno);
            return -1;
        }

        if (fwrite(p_out_buf, out_size, 1, fp) < 1) {
            ERR_PRINTF("Output_BinaryFile file write error");
            fclose(fp);
            return -1;
        } else {
            fclose(fp);
        }    

        return 0;
    }

    std::string read_file(const std::string& path)
    {
        std::ifstream inputfile(path);
        if (!inputfile.is_open()){
            ERR_PRINTF("File read error[%s]", path.c_str());
            return "";
        }
        std::string read_file_data = std::string((std::istreambuf_iterator<char>(inputfile)), std::istreambuf_iterator<char>());

        inputfile.close();

        return read_file_data;
    }
    
    void mock_default_sleep()
    {
        #define DEFAULT_MOCK_SLEEP_MICRO_SECONDS 500000
        usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    }

    void mock_default_sleep_short()
    {
        #define DEFAULT_MOCK_SLEEP_MICRO_SECONDS_SHORT 100000
        usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS_SHORT);
    }    

    void mock_sleep(int milliseconds)
    {
        usleep(milliseconds * 1000);
    }    
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Mock sequence contoller
//////////////////////////////////////////////////////////////////////////////////////////////////
mock_sequence_controller::mock_sequence_controller() :
    Wasm_Force_Stop_flg_(false)
    ,ConfigurationCallback_done_flg_(false)
{
}
mock_sequence_controller& mock_sequence_controller::GetInstance() {
    static mock_sequence_controller instance_;
    return instance_;
}
bool mock_sequence_controller::IsNeedForceStop()
{
    return Wasm_Force_Stop_flg_;
}
void mock_sequence_controller::SetForceStopNeed(bool flg)
{
    Wasm_Force_Stop_flg_ = flg;
}
bool mock_sequence_controller::IsDoneConfigurationCallback()
{
    return ConfigurationCallback_done_flg_;
}
void mock_sequence_controller::SetConfigurationCallbackDone(bool flg)
{
    ConfigurationCallback_done_flg_ = flg;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Callback function interface
//////////////////////////////////////////////////////////////////////////////////////////////////
Callback_if::Callback_if() :
    exec_env_(0)
    ,instance_(0)
{    
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// EVP
//////////////////////////////////////////////////////////////////////////////////////////////////
EVP_mock::EVP_mock() : 
    ConfigurationCallbackFunction_(NULL)
    ,ConfigurationPrivateData_(0)
    ,Count_EVP_processEvent_(0)
{
}

EVP_mock& EVP_mock::GetInstance() {
    static EVP_mock instance_;
    return instance_;
}

struct EVP_client* EVP_mock::EVP_initialize(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock EVP_initialize");
    return reinterpret_cast<struct EVP_client*>(&EVP_mock::GetInstance());
}

EVP_RESULT EVP_mock::EVP_setConfigurationCallback(wasm_exec_env_t exec_env, struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK cb, void* private_data)
{
    INFO_PRINTF("mock EVP_setConfigurationCallback");
    ConfigurationCallbackFunction_ = cb;
    ConfigurationPrivateData_ = private_data;
    return EVP_OK;
}

EVP_RESULT EVP_mock::EVP_processEvent(wasm_exec_env_t exec_env, struct EVP_client* handle, int timeout_ms)
{
    INFO_PRINTF("mock EVP_processEvent count:%d", Count_EVP_processEvent_ + 1);

    Count_EVP_processEvent_ += 1;
    if(1 != Count_EVP_processEvent_)
    {
        MockUtil::mock_sleep(timeout_ms);
        return EVP_OK;
    }

    if (ConfigurationCallbackFunction_ == NULL) {
        ERR_PRINTF("mock EVP_mock::ProcessEvent callback function unregistered");
        return EVP_OK;
    }

    Callback_if* func_if = new Callback_if();
    func_if->exec_env_ = wasm_runtime_spawn_exec_env(exec_env);
    func_if->instance_ = this;

    pthread_t call_ConfigurationCallback_thread_handle;
    pthread_create(&call_ConfigurationCallback_thread_handle, NULL, EVP_mock::call_ConfigurationCallback, func_if);
    pthread_detach(call_ConfigurationCallback_thread_handle);

    MockUtil::mock_default_sleep();
    return EVP_OK;
}

void *EVP_mock::call_ConfigurationCallback(void *arg){
    INFO_PRINTF("mock (internal function) call_ConfigurationCallback");

    MockUtil::mock_default_sleep_short();

    // Getting a Wasm Instance and exec_env
    Callback_if* func_if = reinterpret_cast<Callback_if*>(arg);
    EVP_mock* evp_mock = reinterpret_cast<EVP_mock*>(func_if->instance_);
    wasm_exec_env_t  exec_env = func_if->exec_env_;

    JSON_Value *o_ppl_parameter_value;

    static const char* PARAMETER_JSON_FILE = "ppl_parameter.json";

    std::string ppl_parameter_file_path = MockUtil::get_loader_path() + "/" + PARAMETER_JSON_FILE;
    std::string o_ppl_parameter = MockUtil::read_file(ppl_parameter_file_path);

    // The following process is for checking the JSON type. Do not parse when passing to WASM.
    o_ppl_parameter_value = json_parse_string(o_ppl_parameter.c_str());

    if (json_value_get_type(o_ppl_parameter_value) != JSONObject) {
        WARN_PRINTF("ERROR: ppl command parameters json parse error. please check json file");  
        mock_sequence_controller::GetInstance().SetForceStopNeed(true);
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        delete func_if;
        pthread_exit(NULL);
    }

    // Getting a Wasm Instance
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    char *evp_mock_data = NULL;
    unsigned int evp_mock_data_length = 0;
    uint32_t evp_mock_data_for_wasm = 0;

    char *evp_topic_data = NULL;
    uint32_t evp_topic_for_wasm = 0;

    // Unknown parameter uint32_t handle.

    evp_mock_data_length = (o_ppl_parameter.size() + 1) * sizeof(char);
    evp_mock_data_for_wasm = wasm_runtime_module_malloc(inst, evp_mock_data_length, (void **)&evp_mock_data);
    if (evp_mock_data_for_wasm == 0) {
        ERR_PRINTF("ERROR: evp_mock_data malloc failed");  
        mock_sequence_controller::GetInstance().SetForceStopNeed(true);
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        delete func_if;
        pthread_exit(NULL);
    }
    memcpy(evp_mock_data, o_ppl_parameter.c_str(), evp_mock_data_length);

    char topic[] = "topic";
    evp_topic_for_wasm = wasm_runtime_module_malloc(inst, (strlen(topic) + 1) * sizeof(char), (void **)&evp_topic_data);
    if (evp_topic_for_wasm == 0) {
        ERR_PRINTF("ERROR: evp_topic malloc failed");
        wasm_runtime_module_free(inst, evp_mock_data_for_wasm);  
        mock_sequence_controller::GetInstance().SetForceStopNeed(true);
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        delete func_if;
        pthread_exit(NULL);
    }
    memcpy(evp_topic_data, topic, (strlen(topic) + 1) * sizeof(char));

    // UserData passes privateData received in EVP_setConfigurationCallback 

    uint32_t argv[] = { evp_topic_for_wasm, 
                        evp_mock_data_for_wasm, 
                        evp_mock_data_length, 
                        reinterpret_cast<uint32_t>(evp_mock->ConfigurationPrivateData_)}; 

    DBG_PRINTF("ConfigurationCallback start");
    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)evp_mock->ConfigurationCallbackFunction_, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("ConfigurationCallback failed");
    }

    wasm_runtime_module_free(inst, evp_mock_data_for_wasm);
    wasm_runtime_module_free(inst, evp_topic_for_wasm);

    mock_sequence_controller::GetInstance().SetConfigurationCallbackDone(true);

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback end");

    wasm_runtime_destroy_spawned_exec_env(exec_env);
    delete func_if;
    pthread_exit(NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// SensCord
//////////////////////////////////////////////////////////////////////////////////////////////////
Senscord_mock::Senscord_mock() : 
    FrameRecvCallbackFunction_(NULL)
    ,private_data_(NULL)
    ,stream_(NULL)
    ,SensCord_error_cause_(SENSCORD_ERROR_NONE)
{
}

Senscord_mock& Senscord_mock::GetInstance() {
    static Senscord_mock instance_;
    return instance_;
}

int32_t Senscord_mock::senscord_core_init(wasm_exec_env_t exec_env, senscord_core_t* core)
{
    INFO_PRINTF("mock senscord_core_init");
    return 0;
}

int32_t Senscord_mock::senscord_core_exit(wasm_exec_env_t exec_env, senscord_core_t core)
{
    INFO_PRINTF("mock senscord_core_exit");
    return 0;
}

int32_t Senscord_mock::senscord_core_open_stream(wasm_exec_env_t exec_env, senscord_core_t core, const char* stream_key, senscord_stream_t* stream)
{
    INFO_PRINTF("mock senscord_core_open_stream");

    // read testapp_configuration.json
    const char *SENSCORD_DEFAULT_PROPERTY_FILE = "testapp_configuration.json";

    std::string property_file_path = MockUtil::get_loader_path() + "/" + SENSCORD_DEFAULT_PROPERTY_FILE;
    std::string property_string = MockUtil::read_file(property_file_path);

    if (property_string == ""){
        return -1;
    }

    // json parse root
    JSON_Value* property_data_all = json_parse_string(property_string.c_str());
    // type check
    if (json_value_get_type(property_data_all) != JSONObject) {
        WARN_PRINTF("ERROR: testapp configuration json parse error. please check testapp_configuration.json");
        if(property_data_all != NULL) json_value_free(property_data_all);
        return -1;
    }

    crop_property_.left = json_object_dotget_number(json_object(property_data_all), "input_tensor_dewarp_crop_property.left");
    crop_property_.top = json_object_dotget_number(json_object(property_data_all), "input_tensor_dewarp_crop_property.top");
    crop_property_.width = json_object_dotget_number(json_object(property_data_all), "input_tensor_dewarp_crop_property.width");
    crop_property_.height = json_object_dotget_number(json_object(property_data_all), "input_tensor_dewarp_crop_property.height");

    crop_default_.left = crop_property_.left;
    crop_default_.top = crop_property_.top;
    crop_default_.width = crop_property_.width;
    crop_default_.height = crop_property_.height;

    dnn_property_.network_id = json_object_dotget_number(json_object(property_data_all), "senscord_dnn_property.network_id");
    dnn_property_.network_ordinal = json_object_dotget_number(json_object(property_data_all), "senscord_dnn_property.network_ordinal");

    json_value_free(property_data_all);

    // read output_tensor.jsonc
    const char *OUTPUT_TENSOR_FILE = "output_tensor.jsonc";

    std::string file_path = MockUtil::get_loader_path() + "/" + OUTPUT_TENSOR_FILE;
    std::string output_tensor_data = MockUtil::read_file(file_path);

    if (output_tensor_data == ""){
        return -1;
    }

    // json parse root
    JSON_Value* o_tensor_all = json_parse_string_with_comments(output_tensor_data.c_str());
    // type check
    if (json_value_get_type(o_tensor_all) != JSONArray) {
        WARN_PRINTF("ERROR: output tensor json parse error. please check json file1");
        if(o_tensor_all != NULL) json_value_free(o_tensor_all);
        return -1;
    }

    // Fetch frame data from JSON and create frame data
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    // create stream
    stream_mock* stream_data = new stream_mock();

    // Keep it in a float array
    JSON_Array* o_tensor_frame_array = json_value_get_array(o_tensor_all);
    size_t o_tensor_frame_array_cnt = json_array_get_count(o_tensor_frame_array);
    for (size_t i = 0; i < o_tensor_frame_array_cnt; i++){
        JSON_Value* o_tensor_frame = json_array_get_value(o_tensor_frame_array, i);

        // type check
        if (json_value_get_type(o_tensor_frame) != JSONArray) {
            WARN_PRINTF("ERROR: output tensor json parse error. please check json file2");
            if(o_tensor_all != NULL) json_value_free(o_tensor_all);
            senscord_core_close_stream(exec_env, core, reinterpret_cast<senscord_stream_t>(stream_data));
            return -1;
        }

        JSON_Array* o_tensor_float_array = json_value_get_array(o_tensor_frame);
        size_t o_tensor_float_array_cnt = json_array_get_count(o_tensor_float_array);

        std::vector<float> float_vector;
        for (int k = 0; k < o_tensor_float_array_cnt; k++) {
            double o_tensor_data = json_array_get_number(o_tensor_float_array, k); 
            float_vector.push_back(o_tensor_data);
        }

        // ----------------------------------------
        // create raw data
        // ----------------------------------------
        struct senscord_raw_data_t *raw_data(NULL);
        uint32_t raw_data_for_wasm  = wasm_runtime_module_malloc(inst, sizeof(struct senscord_raw_data_t), (void **)&raw_data);
        if (raw_data_for_wasm == 0){
            ERR_PRINTF("ERROR: raw_data malloc failed");
            if(o_tensor_all != NULL) json_value_free(o_tensor_all);
            senscord_core_close_stream(exec_env, core, reinterpret_cast<senscord_stream_t>(stream_data));
            return -1;
        }

        // Assemble raw_data 
        float* float_data(NULL);
        uint32_t float_data_for_wasm = wasm_runtime_module_malloc(inst, sizeof(float) * float_vector.size(), (void **)&float_data);
        if (float_data_for_wasm == 0){
            ERR_PRINTF("ERROR: float_data malloc failed");
            if(o_tensor_all != NULL) json_value_free(o_tensor_all);
            senscord_core_close_stream(exec_env, core, reinterpret_cast<senscord_stream_t>(stream_data));
            return -1;
        }
        memcpy(float_data, float_vector.data(), sizeof(float) * float_vector.size());

        const char* type = "mock_data_type";
        uint32_t type_length = strlen(type) + 1;

        char* raw_data_type(NULL);
        uint32_t raw_data_type_for_wasm  = wasm_runtime_module_malloc(inst, strlen(type) + 1, (void **)&raw_data_type);
        if (raw_data_type_for_wasm == 0){
            ERR_PRINTF("ERROR: raw_data_type malloc failed");
            if(o_tensor_all != NULL) json_value_free(o_tensor_all);
            senscord_core_close_stream(exec_env, core, reinterpret_cast<senscord_stream_t>(stream_data));
            return -1;
        }
        memcpy(raw_data_type, type, type_length);

        raw_data->address = (void *)float_data_for_wasm;
        raw_data->size = sizeof(float) * float_vector.size();
        raw_data->type = (char *)raw_data_type_for_wasm;
        raw_data->timestamp = 0;

        // ----------------------------------------
        // create channel
        // ----------------------------------------
        channel_mock* channel_data = new channel_mock();
        channel_data->id_ = 1;
        channel_data->raw_data_ = raw_data;

        // ----------------------------------------
        // create frame
        // ----------------------------------------
        frame_mock* frame_data = new frame_mock();
        frame_data->channel_cnt_ = 1;
        frame_data->channel_ = channel_data;

        // ----------------------------------------
        // add frame to stream
        // ----------------------------------------
        stream_data->frame_list_.push_back(frame_data);
    }

    json_value_free(o_tensor_all);

    stream_data->itr_ = stream_data->frame_list_.begin();

    senscord_stream_t* stream_native = reinterpret_cast<senscord_stream_t*>(wasm_runtime_addr_app_to_native(inst,reinterpret_cast<uint32_t>(stream)));
    *stream_native = reinterpret_cast<senscord_stream_t>(stream_data);

    return 0;
}

int32_t Senscord_mock::senscord_core_close_stream(wasm_exec_env_t exec_env, senscord_core_t core, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_core_close_stream");

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    stream_mock* stream_data = reinterpret_cast<stream_mock*>(stream);

    if(!stream_data) {
        return 0;
    }

    for (auto frame : stream_data->frame_list_) {
        if(!frame->channel_) {
            continue;
        }

        // free raw data
        if(frame->channel_->raw_data_) {
            wasm_runtime_module_free(inst, reinterpret_cast<uint32_t>(frame->channel_->raw_data_->address));
            wasm_runtime_module_free(inst, reinterpret_cast<uint32_t>(frame->channel_->raw_data_->type));
            wasm_runtime_module_free(inst,wasm_runtime_addr_native_to_app(inst,frame->channel_->raw_data_));
        }

        // delete channel
        delete frame->channel_;

        // delete frame
        delete frame;
    }
    //delete stream
    delete stream_data;

    return 0;
}

int32_t Senscord_mock::senscord_stream_start(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_stream_start");
    return 0;
}

int32_t Senscord_mock::senscord_stream_stop(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_stream_stop");
    return 0;
}

int32_t Senscord_mock::senscord_stream_get_frame(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t* frame, int32_t timeout_msec)
{
    INFO_PRINTF("mock senscord_stream_get_frame");

    SensCord_error_cause_ = SENSCORD_ERROR_NONE;

    if (mock_sequence_controller::GetInstance().IsDoneConfigurationCallback() != true) {
        DBG_PRINTF("mock senscord_stream_get_frame !ConfigurationCallback_done_flg !!!!\n");
        SensCord_error_cause_ = SENSCORD_ERROR_TIMEOUT;
        return -1;
    }
    stream_mock* stream_data = reinterpret_cast<stream_mock*>(stream);

    // Check frame end. 
    if (stream_data->frame_list_.empty()) {
        SensCord_error_cause_ = SENSCORD_ERROR_UNKNOWN;
        return -1;       
    }    
    if (stream_data->itr_ == stream_data->frame_list_.end()) {
        SensCord_error_cause_ = SENSCORD_ERROR_UNKNOWN;
        return -1;       
    }

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    senscord_frame_t* frame_native = reinterpret_cast<senscord_frame_t*>(wasm_runtime_addr_app_to_native(inst,reinterpret_cast<uint32_t>(frame)));
    *frame_native = reinterpret_cast<senscord_frame_t>(*stream_data->itr_);

    // increment
    ++stream_data->itr_;

    return 0;
}

int32_t Senscord_mock::senscord_stream_release_frame(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t frame)
{
    INFO_PRINTF("mock senscord_stream_release_frame");
    return 0;
}

int32_t Senscord_mock::senscord_stream_get_property(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, void* value, size_t value_size)
{
    INFO_PRINTF("mock senscord_stream_get_property");
    MockUtil::mock_default_sleep();

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    const char* p_property_key = reinterpret_cast<const char*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(property_key)));

    if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DEWARP_CROP) {
        if (value_size != sizeof(input_tensor_dewarp_crop_property)){
            return -1;
        }

        input_tensor_dewarp_crop_property* p_value;
        p_value = reinterpret_cast<input_tensor_dewarp_crop_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));   
        *p_value = crop_property_;

    } else if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DNN) {
        if (value_size != sizeof(senscord_dnn_property)){
            return -1;
        }

        senscord_dnn_property* p_value;
        p_value = reinterpret_cast<senscord_dnn_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));
        *p_value = dnn_property_;

    } else {
        // other property_key, return failure
        return -1;
    }
    
    return 0;
}

int32_t Senscord_mock::senscord_stream_set_property(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, const void* value, size_t value_size)
{
    INFO_PRINTF("mock senscord_stream_set_property");

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    const char* p_property_key = reinterpret_cast<const char*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(property_key)));

    if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DEWARP_CROP) {
        if (value_size != sizeof(input_tensor_dewarp_crop_property)){
            return -1;
        }
        
        input_tensor_dewarp_crop_property* p_value;
        p_value = reinterpret_cast<input_tensor_dewarp_crop_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));
        crop_property_ = *p_value;
        
    } else if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DNN) {
        if (value_size != sizeof(senscord_dnn_property)){
            return -1;
        }

        senscord_dnn_property* p_value;
        p_value = reinterpret_cast<senscord_dnn_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));
        dnn_property_ = *p_value;
        crop_property_ = crop_default_;

    } else {
        // other property_key, return failure
        return -1;
    }

    return 0;
}

int32_t Senscord_mock::senscord_stream_register_frame_callback(wasm_exec_env_t exec_env, senscord_stream_t stream, const senscord_frame_received_callback callback, void* private_data)
{
    INFO_PRINTF("mock senscord_stream_register_frame_callback");

    FrameRecvCallbackFunction_ = callback;
    stream_ = stream;
    private_data_ = private_data;

    call_FrameRecvCallback(exec_env);

    MockUtil::mock_default_sleep();
    return 0;
}

int32_t Senscord_mock::senscord_stream_unregister_frame_callback(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_stream_unregister_frame_callback");

    FrameRecvCallbackFunction_ = NULL;

    return 0;
}

int32_t Senscord_mock::senscord_frame_get_channel_from_channel_id(wasm_exec_env_t exec_env, senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel)
{
    INFO_PRINTF("mock senscord_frame_get_channel_from_channel_id");

    frame_mock* frame_data = reinterpret_cast<frame_mock*>(frame);

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    senscord_channel_t* channel_native = reinterpret_cast<senscord_channel_t*>(wasm_runtime_addr_app_to_native(inst,reinterpret_cast<uint32_t>(channel)));
    *channel_native = reinterpret_cast<senscord_channel_t>(frame_data->channel_);

    MockUtil::mock_default_sleep();
    return 0;
}

int32_t Senscord_mock::senscord_channel_get_raw_data(wasm_exec_env_t exec_env, senscord_channel_t channel, struct senscord_raw_data_t* raw_data)
{
    INFO_PRINTF("mock senscord_channel_get_raw_data");

    channel_mock* channel_data = reinterpret_cast<channel_mock*>(channel);

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    senscord_raw_data_t* raw_data_native = reinterpret_cast<senscord_raw_data_t*>(wasm_runtime_addr_app_to_native(inst,reinterpret_cast<uint32_t>(raw_data)));
    *raw_data_native = *channel_data->raw_data_;

    return 0;
}

int32_t Senscord_mock::senscord_channel_get_property(wasm_exec_env_t exec_env, senscord_channel_t channel, const char* property_key, void* value, size_t value_size)
{
    INFO_PRINTF("mock senscord_channel_get_property");
    MockUtil::mock_default_sleep();

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    const char* p_property_key = reinterpret_cast<const char*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(property_key)));

    if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DEWARP_CROP) {
        if (value_size != sizeof(input_tensor_dewarp_crop_property)){
            return -1;
        }

        input_tensor_dewarp_crop_property* p_value;
        p_value = reinterpret_cast<input_tensor_dewarp_crop_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));
        *p_value = crop_property_;

    } else if (std::string(p_property_key) == SENSCORD_PROPERTY_KEY_DNN) {
        if (value_size != sizeof(senscord_dnn_property)){
            return -1;
        }

        senscord_dnn_property* p_value;
        p_value = reinterpret_cast<senscord_dnn_property*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(value)));
        *p_value = dnn_property_;

    } else {
        // other property_key, return failure
        return -1;
    }

    return 0;
}

int32_t Senscord_mock::senscord_get_last_error(wasm_exec_env_t exec_env, struct senscord_status_t* status)
{
    INFO_PRINTF("mock senscord_get_last_error");

    //Perform address translation for status, then set status->cause
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    senscord_status_t *tmp_status = reinterpret_cast<senscord_status_t*>(wasm_runtime_addr_app_to_native(inst, (uint32_t)status));

    if (mock_sequence_controller::GetInstance().IsNeedForceStop()) {
        tmp_status->cause = SENSCORD_ERROR_UNKNOWN;
        tmp_status->level = SENSCORD_LEVEL_FATAL;
        tmp_status->message = nullptr;
        tmp_status->block = nullptr;
    } else {
        tmp_status->cause = static_cast<senscord_error_cause_t>(SensCord_error_cause_);
        if (tmp_status->cause == SENSCORD_ERROR_UNKNOWN) {
            tmp_status->level = SENSCORD_LEVEL_FATAL;
        } else {
            tmp_status->level = SENSCORD_LEVEL_UNDEFINED;
        }
        tmp_status->message = nullptr;
        tmp_status->block = nullptr;
    }

    DBG_PRINTF("senscord_get_last_error_wrapper status.cause %d", tmp_status->cause);

    return 0;
}

void Senscord_mock::call_FrameRecvCallback(wasm_exec_env_t exec_env){   
    INFO_PRINTF("mock (internal function) call_FrameRecvCallback start");

    if (FrameRecvCallbackFunction_ == NULL) {
        ERR_PRINTF("mock (internal function) call_FrameRecvCallback callback function unregistered");
        return;
    }    

    uint32_t argv[] = { reinterpret_cast<uint32_t>(stream_), 
                        reinterpret_cast<uint32_t>(private_data_)};        

    DBG_PRINTF("FrameRecvCallback start");
    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)FrameRecvCallbackFunction_, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("FrameRecvCallback failed");
    }

    DBG_PRINTF("mock (internal function) call_FrameRecvCallback end");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Sess
//////////////////////////////////////////////////////////////////////////////////////////////////
Sess_mock::Sess_mock() : 
    callback_(NULL)
    ,private_data_(NULL)
    ,data_(0)
    ,output_cnt_(0)
{
}

Sess_mock& Sess_mock::GetInstance() {
    static Sess_mock instance_;
    return instance_;
}

SessResult Sess_mock::SessInit(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessInit");
    return kSessOK;
}

SessResult Sess_mock::SessExit(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessExit");
    return kSessOK;
}

SessResult Sess_mock::SessSendData(wasm_exec_env_t exec_env, const void *data, size_t size, uint64_t timestamp)
{
    INFO_PRINTF("mock SessSendData start");
    const char *OUTPUT_BINARY_FILE_1 = "serialized_result";
    const char *OUTPUT_BINARY_FILE_2 = ".bin";
    const int BUF_LEN = 5;

    char buf[BUF_LEN];        

    snprintf(buf, BUF_LEN, "%02d", ++output_cnt_);
    std::string count_buf(buf);

    std::string file_path = MockUtil::get_loader_path() + "/" + OUTPUT_BINARY_FILE_1 + "_" + count_buf + OUTPUT_BINARY_FILE_2;                     

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    char *databuf = reinterpret_cast<char*>(wasm_runtime_addr_app_to_native(inst, reinterpret_cast<uint32_t>(data)));

    DBG_PRINTF("mock SessSendData datasize: %zu", size);

    MockUtil::Output_BinaryFile(databuf, size, file_path.c_str());

    if (callback_ == NULL) {
        ERR_PRINTF("mock (internal function) call_SendDataDoneCallback_thread callback function unregistered");
        return kSessOK;
    }

    Callback_if* func_if = new Callback_if();
    func_if->exec_env_ = wasm_runtime_spawn_exec_env(exec_env);
    func_if->instance_ = this;

    data_ = (void *)data;

    pthread_t call_SendDataDoneCallback_thread_handle;
    pthread_create(&call_SendDataDoneCallback_thread_handle, NULL, call_SendDataDoneCallback, func_if);
    pthread_join(call_SendDataDoneCallback_thread_handle, NULL);

    delete func_if;
    DBG_PRINTF("mock SessSendData end");

    return kSessOK;
}

SessResult Sess_mock::SessRegisterSendDataCallback(wasm_exec_env_t exec_env, SessSendDataCallback cb, void *private_data)
{
    INFO_PRINTF("mock SessRegisterSendDataCallback");
    callback_ = cb;
    private_data_ = private_data;
    return kSessOK;
}

SessResult Sess_mock::SessUnregisterSendDataCallback(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessUnregisterSendDataCallback");
    callback_ = NULL;
    private_data_ = NULL;
    return kSessOK;
}

void *Sess_mock::call_SendDataDoneCallback(void *arg){
    INFO_PRINTF("mock (internal function) call_SendDataDoneCallback start");

    // Getting a Wasm Instance and exec_env
    Callback_if* func_if = reinterpret_cast<Callback_if*>(arg);
    Sess_mock* sess_mock = reinterpret_cast<Sess_mock*>(func_if->instance_);
    wasm_exec_env_t exec_env = func_if->exec_env_;

    SessResult send_data_ret = kSessOK;
    uint32_t argv[] = { reinterpret_cast<uint32_t>(sess_mock->data_), 
                        reinterpret_cast<uint32_t>(sess_mock->private_data_), 
                        send_data_ret};  

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback SendDataDoneCallback start");

    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)sess_mock->callback_, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("SendDataDoneCallback failed");
    }

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback end");

    wasm_runtime_destroy_spawned_exec_env(exec_env);
    sess_mock->data_ = 0;
    pthread_exit(NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Memory_dump
//////////////////////////////////////////////////////////////////////////////////////////////////
TESTAPP_Memory_dump::TESTAPP_Memory_dump()
{
}

TESTAPP_Memory_dump& TESTAPP_Memory_dump::GetInstance() {
    static TESTAPP_Memory_dump instance_;
    return instance_;
}

void TESTAPP_Memory_dump::TESTAPP_dump_mem_consumption(wasm_exec_env_t exec_env)
{    
    wasm_runtime_dump_mem_consumption(exec_env);
    return;
}

