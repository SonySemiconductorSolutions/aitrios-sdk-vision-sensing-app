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

#ifndef _DEV_MOCK_IF_H_
#define _DEV_MOCK_IF_H_

#include "wasm_export.h"
#include "wasm_runtime_common.h"
#include "vision_app_public.h"

#ifdef __cplusplus
extern "C" {
#endif

struct EVP_client* EVP_initialize_wrapper(wasm_exec_env_t exec_env);
EVP_RESULT EVP_setConfigurationCallback_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK  cb, void* private_data);
EVP_RESULT EVP_processEvent_wrapper(wasm_exec_env_t exec_env, struct EVP_client* handle, int timeout_ms);
int32_t senscord_core_init_wrapper(wasm_exec_env_t exec_env, senscord_core_t* core);
int32_t senscord_core_exit_wrapper(wasm_exec_env_t exec_env, senscord_core_t core);
int32_t senscord_core_open_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, const char* stream_key, senscord_stream_t* stream);
int32_t senscord_core_close_stream_wrapper(wasm_exec_env_t exec_env, senscord_core_t core, senscord_stream_t stream);
int32_t senscord_stream_start_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream);
int32_t senscord_stream_stop_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream);
int32_t senscord_stream_get_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream_app, senscord_frame_t* frame_app, int32_t timeout_msec);
int32_t senscord_stream_release_frame_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_t frame);
int32_t senscord_stream_get_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, void* value, size_t value_size);
int32_t senscord_stream_set_property_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, const char* property_key, const void* value, size_t value_size);
int32_t senscord_stream_register_frame_callback_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream, senscord_frame_received_callback callback, void* private_data);
int32_t senscord_stream_unregister_frame_callback_wrapper(wasm_exec_env_t exec_env, senscord_stream_t stream);
int32_t senscord_frame_get_channel_from_channel_id_wrapper(wasm_exec_env_t exec_env, senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel);
int32_t senscord_channel_get_raw_data_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, struct senscord_raw_data_t* raw_data_app);
int32_t senscord_channel_get_property_wrapper(wasm_exec_env_t exec_env, senscord_channel_t channel_app, const char* property_key_app, void* value_app, size_t value_size);
int32_t senscord_get_last_error_wrapper(wasm_exec_env_t exec_env, struct senscord_status_t* status);
SessResult SessInit_wrapper(wasm_exec_env_t exec_env);
SessResult SessExit_wrapper(wasm_exec_env_t exec_env);
SessResult SessSendData_wrapper(wasm_exec_env_t exec_env, const void *data, size_t size, uint64_t timestamp);
SessResult SessRegisterSendDataCallback_wrapper(wasm_exec_env_t exec_env, SessSendDataCallback cb, void *private_data);
SessResult SessUnregisterSendDataCallback_wrapper(wasm_exec_env_t exec_env);
void TESTAPP_dump_mem_consumption_wrapper(wasm_exec_env_t exec_env);

#ifdef __cplusplus
}
#endif

#endif /* end of _DEV_MOCK_IF_H_ */