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

#include "dev_mock_if.h"

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
    REG_NATIVE_FUNC(senscord_stream_set_property, "(iiii)i"),
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
    REG_NATIVE_FUNC(SessUnregisterSendDataCallback, "()i"),
    REG_NATIVE_FUNC(TESTAPP_dump_mem_consumption, "()")
};

uint32_t
get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols)
{
    *p_module_name = "env";
    *p_native_symbols = native_symbols;
    return sizeof(native_symbols) / sizeof(NativeSymbol);
}
