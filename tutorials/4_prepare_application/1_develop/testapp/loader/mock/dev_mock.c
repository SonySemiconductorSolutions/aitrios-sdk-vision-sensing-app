/****************************************************************************
 * Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * s
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

#include "wasm_export.h"
#include "wasm_runtime_common.h"

#include "vision_app_public.h"

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define ERR_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define WARN_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define INFO_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")
#define DBG_PRINTF(fmt, ...)
#define VER_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__);printf("\n")

#define DEFAULT_MOCK_SLEEP_MICRO_SECONDS 500000

///// EVP mock /////
// Internal passing frame,channel structure
struct senscord_channel_struct_t{
    int channel_id;
    uint32_t raw_data;
};

struct senscord_frame_struct_t{
    int channel_cnt;
    uint32_t channel_data;
};

// Callback function pointer (WASM)
static EVP_CONFIGURATION_CALLBACK ConfigurationCallbackFunction = NULL;
static SessSendDataCallback SendDataDoneCallbackFunction = NULL;
static senscord_frame_received_callback FrameRecvCallbackFunction = NULL;

// ConfigurationCallback confirmed processed
static int ConfigurationCallback_done_flg = false;

// WASM handover area pointer
static uint32_t private_data_conf_addr;
static uint32_t private_data_addr;
static uint32_t stream_addr;
static uint32_t private_data_frame_addr;

static int Count_EVP_processEvent = 0;

static int SensCord_error_cause = SENSCORD_ERROR_NONE;

// SendData Cnt
static int output_cnt = 0;

// Wasm force stop
static bool Wasm_Force_Stop_flg = false;

// prototype
static void get_loader_path(char *, int);
static int Output_BinaryFile(void *, uint32_t, char*);
static void *call_ConfigurationCallback(void *);
static void *call_SendDataDoneCallback(void *);
static void call_FrameRecvCallback(wasm_exec_env_t);
static void call_SendDataDoneCallback_thread(wasm_exec_env_t, uint32_t);
static void call_ConfigurationCallback_thread(wasm_exec_env_t, struct EVP_client *);
static void free_wasm_data(wasm_exec_env_t, senscord_frame_t);

static
struct EVP_client* EVP_initialize_wrapper(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock EVP_initialize");

    ConfigurationCallbackFunction = NULL;

    return NULL;
}

static
int EVP_setConfigurationCallback_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK  cb, void* private_data)
{
    INFO_PRINTF("mock EVP_setConfigurationCallback");

    ConfigurationCallbackFunction = cb;
    private_data_conf_addr = (uint32_t)private_data;

    return 0;
}

// Get the path where the loader is located.
// The path storage area must be prepared by the caller.
static void get_loader_path(char *loader_path, int path_size)
{
    const size_t LINKSIZE = 100;
    char link[LINKSIZE];

    snprintf(link, LINKSIZE, "/proc/%d/exe", getpid() );
    readlink(link, loader_path, path_size);
    char* pos = strrchr(loader_path, '/');
    *pos = '\0';
}

static void call_ConfigurationCallback_thread(wasm_exec_env_t exec_env, struct EVP_client* handle)
{
    pthread_t call_ConfigurationCallback_thread_handle;

    INFO_PRINTF("mock (internal function) call_ConfigurationCallback_thread");

    if (ConfigurationCallbackFunction == NULL) {
        ERR_PRINTF("mock (internal function) call_ConfigurationCallback_thread callback function unregistered");
        return;
    }
  
    wasm_exec_env_t new_exec_env = wasm_runtime_spawn_exec_env(exec_env);

    pthread_create(&call_ConfigurationCallback_thread_handle, NULL, call_ConfigurationCallback, new_exec_env);
    pthread_detach(call_ConfigurationCallback_thread_handle);

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback_thread end");
}

static char* PARAMETER_JSON_FILE = "ppl_parameter.json";

static void *call_ConfigurationCallback(void *arg){
    char *evp_mock_data = NULL;
    unsigned int evp_mock_data_length = 0;
    uint32_t evp_mock_data_for_wasm = 0;

    char *evp_topic_data = NULL;
    uint32_t evp_topic_for_wasm = 0;

    INFO_PRINTF("mock (internal function) call_ConfigurationCallback");

    usleep(100000);

    // Getting a Wasm Instance
    wasm_exec_env_t exec_env = arg;

    FILE *ppl_parameter_fp;
    long file_size;
    size_t read_size;
    char *o_ppl_parameter;
    JSON_Value *o_ppl_parameter_value;

    char *file_path;
    size_t path_size = sizeof(char) * PATH_MAX;

    file_path = (char *)malloc(path_size);
    memset(file_path, '\0', path_size);
    get_loader_path(file_path, path_size);
    snprintf(file_path + strnlen(file_path, path_size), path_size - strnlen(file_path, path_size) - 1
                , "/%s", PARAMETER_JSON_FILE);

    int tmp_errno = 0;
    ppl_parameter_fp = fopen(file_path, "r");        
    tmp_errno = errno;
 
    if (ppl_parameter_fp == NULL) {
        ERR_PRINTF("ERROR: PPL_parameter File error errno: %d filepath: %s", tmp_errno, file_path);
        free(file_path);
        file_path = NULL;
        Wasm_Force_Stop_flg = true;
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        pthread_exit(NULL);
    }

    // Get PPL parameter
    fseek(ppl_parameter_fp , 0, SEEK_END);
    file_size = ftell(ppl_parameter_fp);
    rewind(ppl_parameter_fp);

    o_ppl_parameter = (char *)malloc(sizeof(char) * (file_size + 1));
    read_size = fread(o_ppl_parameter , sizeof(char), file_size, ppl_parameter_fp);
    o_ppl_parameter[file_size] = '\0';
    if (file_size != (long)read_size) {
        ERR_PRINTF("ERROR: PPL_parameter File error filepath: %s", file_path);
        free(o_ppl_parameter);
        o_ppl_parameter = NULL;
        fclose(ppl_parameter_fp);
        free(file_path);
        file_path = NULL;        
        Wasm_Force_Stop_flg = true;
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        pthread_exit(NULL);
    }
    fclose(ppl_parameter_fp);

    free(file_path);
    file_path = NULL;    

    o_ppl_parameter_value = json_parse_string(o_ppl_parameter);

    if (json_value_get_type(o_ppl_parameter_value) != JSONObject) {
        WARN_PRINTF("ERROR: ppl command parameters json parse error. please check json file");
        free(o_ppl_parameter);
        o_ppl_parameter = NULL;    
        Wasm_Force_Stop_flg = true;
        wasm_runtime_destroy_spawned_exec_env(exec_env);
        pthread_exit(NULL);
    }

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback get wasm instance");

    // Getting a Wasm Instance
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    // Unknown parameter uint32_t handle.
    unsigned int data_length = file_size + 1;

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback malloc evp_mock_data_for_wasm");

    evp_mock_data_for_wasm = wasm_runtime_module_malloc(inst, data_length, (void **)&evp_mock_data);

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback memcpy evp_mock_data");   
    memcpy(evp_mock_data, o_ppl_parameter, data_length);
    evp_mock_data_length = data_length;

    free(o_ppl_parameter);
    o_ppl_parameter = NULL;

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback malloc evp_topic_for_wasm");

    char topic[] = "topic";
    evp_topic_for_wasm = wasm_runtime_module_malloc(inst, strlen(topic) + 1, (void **)&evp_topic_data);

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback memcpy evp_topic_data");
    memcpy(evp_topic_data, topic, strlen(topic) + 1);

    // UserData passes privateData received in EVP_setConfigurationCallback 

    uint32_t argv[] = { evp_topic_for_wasm, 
                        evp_mock_data_for_wasm, 
                        evp_mock_data_length, 
                        private_data_conf_addr}; 

    DBG_PRINTF("ConfigurationCallback start");
    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)ConfigurationCallbackFunction, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("ConfigurationCallback failed");
    }

    wasm_runtime_module_free(inst, evp_mock_data_for_wasm);
    wasm_runtime_module_free(inst, evp_topic_for_wasm);

    ConfigurationCallback_done_flg = true;     

    DBG_PRINTF("mock (internal function) call_ConfigurationCallback end");

    wasm_runtime_destroy_spawned_exec_env(exec_env);
    pthread_exit(NULL);
}

struct SendDataDoneCallback_parm{
    wasm_exec_env_t exec_env;
    uint32_t data;
};

static void call_SendDataDoneCallback_thread(wasm_exec_env_t exec_env, uint32_t data){
    INFO_PRINTF("mock (internal function) call_SendDataDoneCallback_thread");

    pthread_t call_SendDataDoneCallback_thread_handle;

    if (SendDataDoneCallbackFunction == NULL) {
        ERR_PRINTF("mock (internal function) call_SendDataDoneCallback_thread callback function unregistered");
        return;
    }

    struct SendDataDoneCallback_parm *parm;
    parm = (struct SendDataDoneCallback_parm *)malloc(sizeof(struct SendDataDoneCallback_parm));   

    wasm_exec_env_t new_exec_env = wasm_runtime_spawn_exec_env(exec_env);

    parm->exec_env = new_exec_env;
    parm->data = data;

    pthread_create(&call_SendDataDoneCallback_thread_handle, NULL, call_SendDataDoneCallback, (void *)parm);
    pthread_join(call_SendDataDoneCallback_thread_handle, NULL);

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback_thread end");
}

static void *call_SendDataDoneCallback(void *arg){ 
    INFO_PRINTF("mock (internal function) call_SendDataDoneCallback start");

    // Getting a Wasm Instance
    struct SendDataDoneCallback_parm *parm_tmp;

    parm_tmp = arg;
    wasm_exec_env_t exec_env = parm_tmp->exec_env;
    uint32_t data = parm_tmp->data;

    SessResult send_data_ret = kSessOK;

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback callback parameter set");

    uint32_t argv[] = { data, 
                        private_data_addr, 
                        send_data_ret};  

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback SendDataDoneCallback start");

    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)SendDataDoneCallbackFunction, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("SendDataDoneCallback failed");
    }

    free(parm_tmp);
    parm_tmp = NULL;

    DBG_PRINTF("mock (internal function) call_SendDataDoneCallback end");

    wasm_runtime_destroy_spawned_exec_env(exec_env);
    pthread_exit(NULL);
}

static void call_FrameRecvCallback(wasm_exec_env_t exec_env){   
    INFO_PRINTF("mock (internal function) call_FrameRecvCallback start");

    if (FrameRecvCallbackFunction == NULL) {
        ERR_PRINTF("mmock (internal function) call_FrameRecvCallback callback function unregistered");
        return;
    }    

    uint32_t argv[] = { stream_addr, 
                        private_data_frame_addr};        

    DBG_PRINTF("FrameRecvCallback start");
    if (!wasm_runtime_call_indirect(exec_env, (uint32_t)FrameRecvCallbackFunction, sizeof(argv) / sizeof(uint32_t), argv)) {
        ERR_PRINTF("FrameRecvCallback failed");
    }

    DBG_PRINTF("mock (internal function) call_FrameRecvCallback end");
}

static
int EVP_processEvent_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, int timeout_ms)
{
    INFO_PRINTF("mock EVP_processEvent count:%d", Count_EVP_processEvent + 1);

    Count_EVP_processEvent += 1;

    if(Count_EVP_processEvent == 1)
    {
        call_ConfigurationCallback_thread(exec_env, handle);

    }

    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return 0;
}

///// SENSCORD mock /////

static
int32_t senscord_core_init_wrapper(wasm_exec_env_t exec_env, senscord_core_t* core)
{
    INFO_PRINTF("mock senscord_core_init");
    return 0;
}

static
int32_t senscord_core_exit_wrapper(wasm_exec_env_t exec_env, senscord_core_t core)
{
    INFO_PRINTF("mock senscord_core_exit");
    return 0;
}

static
int32_t senscord_core_open_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, const char* stream_key, senscord_stream_t* stream)
{
    INFO_PRINTF("mock senscord_core_open_stream");
    return 0;
}

static
int32_t senscord_core_close_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_core_close_stream");
    return 0;
}

static
int32_t senscord_stream_start_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_stream_start");
    return 0;
}

static
int32_t senscord_stream_stop_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream)
{
    INFO_PRINTF("mock senscord_stream_stop");
    return 0;
}

const char *OUTPUT_TENSOR_FILE = "output_tensor.jsonc";

static int get_frame_cnt = 0;

static
int32_t senscord_stream_get_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream_app, senscord_frame_t* frame_app, int32_t timeout_msec)
{
    INFO_PRINTF("mock senscord_stream_get_frame");
    SensCord_error_cause = SENSCORD_ERROR_NONE;

    // Assign return values according to execution status ()

    if (ConfigurationCallback_done_flg != true) {
        DBG_PRINTF("mock senscord_stream_get_frame !ConfigurationCallback_done_flg !!!!\n");
        SensCord_error_cause = SENSCORD_ERROR_TIMEOUT;
        return -1;
    }
 
    get_frame_cnt++;

    if (get_frame_cnt != 1) {
        SensCord_error_cause = SENSCORD_ERROR_UNKNOWN;
        return -1;       
    }

    // Creating Frame Data
    FILE *output_tensor_fp;
    long file_size;
    size_t read_size;
    char *o_tensor;
    JSON_Value *o_tensor_value;
    JSON_Array *o_tensor_array;
    size_t o_tensor_data_length;
    float *dummy_data;

    float *mock_data = NULL;
    unsigned int mock_data_length = 0;
    struct senscord_frame_struct_t *frame_data = NULL;
    struct senscord_channel_struct_t *channel_data = NULL;
    struct senscord_raw_data_t *channel_raw_data = NULL;
    char* channel_data_raw_data_type = NULL;
    uint32_t mock_data_for_wasm = 0;
    uint32_t frame_data_for_wasm = 0;
    uint32_t channel_data_for_wasm = 0;
    uint32_t channel_data_raw_data_for_wasm = 0;
    uint32_t channel_data_raw_data_type_for_wasm = 0;

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    char *file_path;
    size_t path_size = sizeof(char) * PATH_MAX;

    file_path = (char *)malloc(path_size);
    memset(file_path, '\0', path_size);
    get_loader_path(file_path, path_size);
    snprintf(file_path + strnlen(file_path, path_size) , path_size - strnlen(file_path, path_size) - 1
                , "/%s", OUTPUT_TENSOR_FILE);

    errno = 0;
    output_tensor_fp = fopen(file_path, "r");
    int errno_tmp = errno;

    if (output_tensor_fp == NULL) {
        ERR_PRINTF("ERROR: Output_tensor File error errno=%d filepath: %s", errno_tmp, file_path);
        free(file_path);
        file_path = NULL;
        SensCord_error_cause = SENSCORD_ERROR_UNKNOWN;
        return -1;
    }

    // Convert float array
    fseek(output_tensor_fp, 0, SEEK_END);
    file_size = ftell(output_tensor_fp);
    rewind(output_tensor_fp);

    o_tensor = (char *)malloc(sizeof(char) * (file_size + 1));
    read_size = fread(o_tensor, sizeof(char), file_size, output_tensor_fp);
    o_tensor[file_size] = '\0';
    if (file_size != (long)read_size) {
        ERR_PRINTF("ERROR: Output_tensor File error filepath: %s", file_path);
        free(o_tensor);
        o_tensor = NULL;
        fclose(output_tensor_fp);
        free(file_path);
        file_path = NULL;        
        SensCord_error_cause = SENSCORD_ERROR_UNKNOWN;
        return -1;
    }
    fclose(output_tensor_fp);

    free(file_path);
    file_path = NULL;   

    o_tensor_value = json_parse_string_with_comments(o_tensor);

    if (json_value_get_type(o_tensor_value) != JSONArray) {
        WARN_PRINTF("ERROR: output tensor json parse error. please check json file");
        free(o_tensor);
        o_tensor = NULL;
        if(o_tensor_value != NULL) json_value_free(o_tensor_value);
        SensCord_error_cause = SENSCORD_ERROR_UNKNOWN;
        return -1;
    }

    o_tensor_array = json_value_get_array(o_tensor_value);

    o_tensor_data_length = json_array_get_count(o_tensor_array);
    if (o_tensor_data_length == 0) {
        WARN_PRINTF("output_tensor data length 0");
        free(o_tensor);
        o_tensor = NULL;
        if(o_tensor_value != NULL) json_value_free(o_tensor_value);
        SensCord_error_cause = SENSCORD_ERROR_UNKNOWN;
        return -1;
    }
    dummy_data = (float *)malloc(sizeof(float) * o_tensor_data_length);
    for (size_t i = 0; i < o_tensor_data_length; i++) {
        double o_tensor_data = json_array_get_number(o_tensor_array, i);
        dummy_data[i] = o_tensor_data;
    }
    unsigned int data_length = sizeof(float) * o_tensor_data_length;
    DBG_PRINTF("senscord_stream_get_raw_data o_tensor_data_length:[%d] data_length:[%d]"
                ,o_tensor_data_length,data_length);

    mock_data_for_wasm = wasm_runtime_module_malloc(inst, data_length, (void **)&mock_data);
    memcpy(mock_data, dummy_data, data_length);
    mock_data_length = data_length;

    senscord_frame_t* frame = wasm_runtime_addr_app_to_native(inst, (uint32_t)frame_app);

    frame_data_for_wasm  = wasm_runtime_module_malloc(inst, sizeof(struct senscord_frame_struct_t), (void **)&frame_data);
    channel_data_for_wasm  = wasm_runtime_module_malloc(inst, sizeof(struct senscord_channel_struct_t), (void **)&channel_data);
    channel_data_raw_data_for_wasm  = wasm_runtime_module_malloc(inst, sizeof(struct senscord_raw_data_t), (void **)&channel_raw_data);

    char* type = "mock_data_type";
    uint32_t type_length = strlen(type) + 1;
    uint64_t dummy_timestamp = 0;

    channel_data_raw_data_type_for_wasm  = wasm_runtime_module_malloc(inst, type_length, (void **)&channel_data_raw_data_type);
    memcpy(channel_data_raw_data_type, type, type_length);
    channel_raw_data->type = (char *)channel_data_raw_data_type_for_wasm;
    channel_raw_data->address = (struct senscord_raw_data_t *)mock_data_for_wasm;
    channel_raw_data->size = mock_data_length;
    channel_raw_data->timestamp = dummy_timestamp;

    channel_data->channel_id = 1;
    channel_data->raw_data = channel_data_raw_data_for_wasm;

    frame_data->channel_cnt = 1;
    frame_data->channel_data = channel_data_for_wasm;

    DBG_PRINTF("mock senscord_stream_get_frame frame_data_for_wasm[%d] channel_data_for_wasm[%d] channel_data_raw_data_for_wasm[%d] channel_data_raw_data_type_for_wasm[%d] mock_data_for_wasm[%d]",
                frame_data_for_wasm, channel_data_for_wasm, channel_data_raw_data_for_wasm, channel_data_raw_data_type_for_wasm, mock_data_for_wasm);

    *frame = (senscord_frame_t)frame_data_for_wasm;

    // Cleanup
    json_value_free(o_tensor_value);
    free(o_tensor);
    o_tensor = NULL;
    free(dummy_data);
    dummy_data = NULL;

    return 0;
}

static
int32_t senscord_stream_release_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t frame)
{
    INFO_PRINTF("mock senscord_stream_release_frame");

    free_wasm_data(exec_env, frame);

    return 0;
}

static
void free_wasm_data(wasm_exec_env_t exec_env, senscord_frame_t frame)
{
    INFO_PRINTF("mock (internal function) free_wasm_data");

    uint32_t mock_data_for_wasm = 0;
    uint32_t frame_data_for_wasm = 0;
    uint32_t channel_data_for_wasm = 0;
    uint32_t channel_data_raw_data_for_wasm = 0;
    uint32_t channel_data_raw_data_type_for_wasm = 0;

    // The last of the flow processing one frame
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    frame_data_for_wasm = (uint32_t)frame;
    DBG_PRINTF("mock (internal function) free_wasm_data address get frame[%d]",frame_data_for_wasm);
    struct senscord_frame_struct_t *frame_tmp = wasm_runtime_addr_app_to_native(inst, (uint32_t)frame);

    channel_data_for_wasm = (uint32_t)frame_tmp->channel_data;
    DBG_PRINTF("mock (internal function) free_wasm_data address get channel[%d]",channel_data_for_wasm);
    struct senscord_channel_struct_t *channel_tmp = wasm_runtime_addr_app_to_native(inst, (uint32_t)channel_data_for_wasm);

    channel_data_raw_data_for_wasm = (uint32_t)channel_tmp->raw_data;
    DBG_PRINTF("mock (internal function) free_wasm_data address get raw_data[%d]",channel_data_raw_data_for_wasm);
    struct senscord_raw_data_t *raw_data_tmp = wasm_runtime_addr_app_to_native(inst, (uint32_t)channel_data_raw_data_for_wasm);

    channel_data_raw_data_type_for_wasm = (uint32_t)raw_data_tmp->type;
    mock_data_for_wasm = (uint32_t)raw_data_tmp->address;
    DBG_PRINTF("mock (internal function) free_wasm_data address get raw_data type[%d],mock_data[%d]",
                channel_data_raw_data_type_for_wasm,mock_data_for_wasm);

    DBG_PRINTF("mock (internal function) free_wasm_data address memory free frame_data");

    if (frame_data_for_wasm != 0) {
        wasm_runtime_module_free(inst, frame_data_for_wasm);
    }

    DBG_PRINTF("mock (internal function) free_wasm_data address memory free channel_data");

    if (channel_data_for_wasm != 0) {
        wasm_runtime_module_free(inst, channel_data_for_wasm);
    }

    DBG_PRINTF("mock (internal function) free_wasm_data address memory free raw_data");

    if (channel_data_raw_data_for_wasm != 0) {
        wasm_runtime_module_free(inst, channel_data_raw_data_for_wasm);
    }

    DBG_PRINTF("mock (internal function) free_wasm_data address memory free raw_data type data");

    if (channel_data_raw_data_type_for_wasm != 0) {
        wasm_runtime_module_free(inst, channel_data_raw_data_type_for_wasm);
    }

    DBG_PRINTF("mock (internal function) free_wasm_data address memory free raw_data mock_data");

    if (mock_data_for_wasm != 0) {
        wasm_runtime_module_free(inst, mock_data_for_wasm);
    }

    return;
}

static
int32_t senscord_stream_get_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, void* value, size_t value_size)
{
    INFO_PRINTF("mock senscord_stream_get_property");

    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return 0;
}

static
int32_t senscord_stream_set_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, const void* value, size_t value_size)
{
    INFO_PRINTF("mock senscord_stream_set_property");

    return 0;
}

static
int32_t senscord_stream_register_frame_callback_wrapper(wasm_exec_env_t exec_env, uint32_t stream, senscord_frame_received_callback callback, uint32_t private_data)
{
    INFO_PRINTF("mock senscord_stream_register_frame_callback");

    FrameRecvCallbackFunction = callback;
    stream_addr = stream;
    private_data_frame_addr = private_data;

    call_FrameRecvCallback(exec_env);

    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return 0;
}

static
int32_t senscord_stream_unregister_frame_callback_wrapper(wasm_exec_env_t exec_env, uint32_t stream)
{
    INFO_PRINTF("mock senscord_stream_unregister_frame_callback");

    FrameRecvCallbackFunction = NULL;

    return 0;
}

static
int32_t senscord_frame_get_channel_from_channel_id_wrapper(wasm_exec_env_t exec_env, senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel)
{
    INFO_PRINTF("mock senscord_frame_get_channel_from_channel_id");
    
    // Passing a frame containing the raw_data to the channel
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    struct senscord_frame_struct_t *frame_tmp = wasm_runtime_addr_app_to_native(inst, (uint32_t)frame);
    senscord_channel_t *channel_addr = wasm_runtime_addr_app_to_native(inst, (uint32_t)channel);

    *channel_addr = (senscord_channel_t)frame_tmp->channel_data;

    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return 0;
}

static
int32_t senscord_channel_get_raw_data_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, struct senscord_raw_data_t* raw_data_app)
{
    INFO_PRINTF("mock senscord_channel_get_raw_data");

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);

    senscord_channel_t channel = wasm_runtime_addr_app_to_native(inst, (uint32_t)channel_app);
    struct senscord_channel_struct_t* channel_raw_data = channel;
    struct senscord_raw_data_t* raw_data_tmp = wasm_runtime_addr_app_to_native(inst, channel_raw_data->raw_data);
    struct senscord_raw_data_t* raw_data = wasm_runtime_addr_app_to_native(inst, (uint32_t)raw_data_app);

    DBG_PRINTF("mock senscord_channel_get_raw_data raw_data.address[%d] raw_data.size[%d]", raw_data_tmp->address, raw_data_tmp->size);

    raw_data->address = raw_data_tmp->address;
    raw_data->size = raw_data_tmp->size;
    raw_data->type = raw_data_tmp->type;
    raw_data->timestamp = raw_data_tmp->timestamp;

    float *raw_data_elm = wasm_runtime_addr_app_to_native(inst, (uint32_t)raw_data_tmp->address);
    
    INFO_PRINTF("mock_data_length %d", raw_data->size);
    INFO_PRINTF("mock_data %x", (int)raw_data->address);
    INFO_PRINTF("mock_data[0] %f", raw_data_elm[0]);
    INFO_PRINTF("mock_data[1] %f", raw_data_elm[1]);

    DBG_PRINTF("mock senscord_channel_get_raw_data end");

    return 0;
}

static
int32_t senscord_channel_get_property_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, const char* property_key_app, void* value_app, size_t value_size)
{
    INFO_PRINTF("mock senscord_channel_get_property");
    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return 0;
}

static
int32_t senscord_get_last_error_wrapper(wasm_exec_env_t exec_env, struct senscord_status_t* status)
{
    INFO_PRINTF("mock senscord_get_last_error");

    //Perform address translation for status, then set status->cause
    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    struct senscord_status_t *tmp_status = wasm_runtime_addr_app_to_native(inst, (uint32_t)status);

    if (Wasm_Force_Stop_flg) {
        tmp_status->cause = SENSCORD_ERROR_UNKNOWN;
    } else {
        tmp_status->cause = SensCord_error_cause;
    }

    DBG_PRINTF("senscord_get_last_error_wrapper status.cause %d", tmp_status->cause);

    return 0;
}

static
SessResult SessInit_wrapper(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessInit");

    return kSessOK;
}

static
SessResult SessExit_wrapper(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessExit");

    return kSessOK;
}

const char *OUTPUT_BINARY_FILE_1 = "serialized_result";
const char *OUTPUT_BINARY_FILE_2 = ".bin";

static
SessResult SessSendData_wrapper(wasm_exec_env_t exec_env, const uint32_t data, size_t size, uint64_t timestamp)
{
    INFO_PRINTF("mock SessSendData start");
        
    char *file_path;
    size_t path_size = sizeof(char) * PATH_MAX;

    file_path = (char *)malloc(path_size);
    memset(file_path, '\0', path_size);
    get_loader_path(file_path, path_size);

    snprintf(file_path + strnlen(file_path, path_size), path_size - strnlen(file_path, path_size) - 1
                , "/%s_%02d%s"
                , OUTPUT_BINARY_FILE_1, ++output_cnt, OUTPUT_BINARY_FILE_2);

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    char *databuf = wasm_runtime_addr_app_to_native(inst, data);

    DBG_PRINTF("SessSendData datasize: %d", size);

    Output_BinaryFile(databuf, size, file_path);

    free(file_path);
    file_path = NULL;

    call_SendDataDoneCallback_thread(exec_env, data);

    DBG_PRINTF("mock SessSendData end");

    return kSessOK;
}

static int Output_BinaryFile(void *p_out_buf, uint32_t out_size, char* p_filename)
{
    FILE* fp;
   
    INFO_PRINTF("mock (internal function) Output_BinaryFile datasize:%d", out_size);
  
    int tmp_errno = 0;
    fp = fopen(p_filename, "wb");
    tmp_errno = errno;

    if (tmp_errno != 0) {
        ERR_PRINTF("Output_BinaryFile file open error errno: %d", tmp_errno);
        return -1;
    } 

    if (fwrite(p_out_buf, out_size, 1, fp) < 1) {
        ERR_PRINTF("Output_BinaryFile file write error");
        return -1;
    } else {
        fclose(fp);
    }    

    return 0;
}

static
SessResult SessRegisterSendDataCallback_wrapper(wasm_exec_env_t exec_env, SessSendDataCallback cb_app, void *private_data_app)
{
    INFO_PRINTF("mock SessRegisterSendDataCallback");

    // Register SendDataDoneCallback
    SendDataDoneCallbackFunction = cb_app;
    private_data_addr = (uint32_t)private_data_app;

    usleep(DEFAULT_MOCK_SLEEP_MICRO_SECONDS);
    return kSessOK;
}

static
SessResult SessUnregisterSendDataCallback_wrapper(wasm_exec_env_t exec_env)
{
    INFO_PRINTF("mock SessUnregisterSendDataCallback");

    SendDataDoneCallbackFunction = NULL;

    return kSessOK;
}

/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(EVP_initialize, "()i"),
    REG_NATIVE_FUNC(EVP_setConfigurationCallback, "(iii)i"),
    REG_NATIVE_FUNC(EVP_processEvent, "(ii)i"),
    REG_NATIVE_FUNC(senscord_core_init, "(i)i"),
    REG_NATIVE_FUNC(senscord_core_exit, "(i)i"),
    REG_NATIVE_FUNC(senscord_core_open_stream, "(iii)i"),
    REG_NATIVE_FUNC(senscord_core_close_stream, "(ii)i"),
    REG_NATIVE_FUNC(senscord_stream_start, "(i)i"),
    REG_NATIVE_FUNC(senscord_stream_stop, "(i)i"),
    REG_NATIVE_FUNC(senscord_stream_get_frame, "(iii)i"),
    REG_NATIVE_FUNC(senscord_stream_release_frame, "(ii)i"),
    REG_NATIVE_FUNC(senscord_stream_get_property, "(iiii)i"),
    REG_NATIVE_FUNC(senscord_stream_set_property, "(iii)i"),
    REG_NATIVE_FUNC(senscord_stream_register_frame_callback, "(iii)i"),
    REG_NATIVE_FUNC(senscord_stream_unregister_frame_callback, "(i)i"),
    REG_NATIVE_FUNC(senscord_frame_get_channel_from_channel_id, "(iii)i"),
    REG_NATIVE_FUNC(senscord_channel_get_raw_data, "(ii)i"),
    REG_NATIVE_FUNC(senscord_channel_get_property, "(iiii)i"),
    REG_NATIVE_FUNC(senscord_get_last_error, "(i)i"),
    REG_NATIVE_FUNC(SessInit, "()i"),
    REG_NATIVE_FUNC(SessExit, "()i"),
    REG_NATIVE_FUNC(SessSendData, "(iiI)i"),
    REG_NATIVE_FUNC(SessRegisterSendDataCallback, "(ii)i"),
    REG_NATIVE_FUNC(SessUnregisterSendDataCallback, "()i")
};
/* clang-format on */

uint32_t
get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols)
{
    *p_module_name = "env";
    *p_native_symbols = native_symbols;
    return sizeof(native_symbols) / sizeof(NativeSymbol);
}
