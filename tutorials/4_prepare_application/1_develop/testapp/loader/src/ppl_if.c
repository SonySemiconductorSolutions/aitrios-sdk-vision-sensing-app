/****************************************************************************
 * Copyright 2022 Sony Semiconductor Solutions Corp. All rights reserved.
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
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include "ppl_if.h"
#include "network_struct.h"
#include "wasm_export.h"

/* Pivate definitions */

// #define ERR_PRINTF(fmt, ...)  DLog(Log_Error, "ScPP", fmt, ##__VA_ARGS__)
// #define INF_PRINTF(fmt, ...)  DLog(Log_Info,  "ScPP", fmt, ##__VA_ARGS__)
// #define DBG_PRINTF(fmt, ...)  DLog(Log_Debug, "ScPP", fmt, ##__VA_ARGS__)
#define ERR_PRINTF(fmt, ...)
#define INF_PRINTF(fmt, ...)
#define DBG_PRINTF(fmt, ...)

/* PPL paramater */

static char *s_ppl_paramater = NULL;
static bool  s_ppl_enable    = true; // false;
static size_t s_ppl_paramater_size = 0;
wasm_module_inst_t sp_ppl_wasm_module_inst = NULL;
wasm_exec_env_t sp_ppl_wasm_exec_env = NULL;

/*----------------------------------------------------------------------*/
EPPL_RESULT_CODE PPL_SetParamater(const char *param, size_t size, bool enable)
{
    DBG_PRINTF("%s", __func__);
    s_ppl_enable = enable;

    if (!enable) {
        return E_PPL_OK;
    }

    if (s_ppl_paramater) {
        free(s_ppl_paramater);
    }

    s_ppl_paramater = NULL;
    s_ppl_paramater_size = 0;

    if (param == NULL) {
        return E_PPL_OK;
    }

    /* Base64 decode */

    s_ppl_paramater = (char *)malloc(size);
    s_ppl_paramater_size = size;

    if (s_ppl_paramater == NULL) {
        ERR_PRINTF("ERROR: memory allocate for base64 decode!");
        return E_PPL_E_MEMORY_ERROR;
    }

    size = b64_decode((uint8_t *)param, size, (uint8_t *)s_ppl_paramater);

    s_ppl_paramater[size] = '\0';

    return E_PPL_OK;
}

int SetupWasmInstance() {
    
}

/*----------------------------------------------------------------------*/
/* PPL_Initialize
 * return: Always return E_PPL_OK. We don't force the user to use PPL wasm.
 *         If failed to load PPL wasm, the system output OutputTensor directly.
 */
EPPL_RESULT_CODE PPL_Initialize(uint32_t network_id)
{
    EPPL_RESULT_CODE ret = E_PPL_OTHER;

    if (!s_ppl_enable) {
        return E_PPL_OK;
    }
    const char *param = "";

    if (s_ppl_paramater) {
        param = s_ppl_paramater;
    }

    INF_PRINTF("PPL_Initialize(network id=0x%06x, param=%s)", network_id, param);

    uint32_t argv[2];
    uint32_t argc = sizeof(argv) / sizeof(uint32_t);
    uint32_t wasm_buffer = 0;
    char* native_buffer = NULL;
    size_t param_len = strlen(param);
    /* param_len_plus_one is the length of param + a null character */
    size_t param_len_plus_one = param_len + 1;

    if (INT_MAX < param_len_plus_one) {
        DBG_PRINTF("param_len_plus_one error: [%d]", param_len_plus_one);
        goto end;
    }

    if (NULL == sp_ppl_wasm_module_inst) {
        DBG_PRINTF("module_inst is NULL.");
        goto end;
    }

    wasm_buffer = wasm_runtime_module_malloc(sp_ppl_wasm_module_inst,
                                             param_len_plus_one,
                                             (void**)&native_buffer);
    if (0 == wasm_buffer) {
        DBG_PRINTF("wasm_buffer is NULL.");
        goto end;
    }
    if (NULL == native_buffer) {
        DBG_PRINTF("native_buffer is NULL.");
        goto end;
    }

    strncpy(native_buffer, param, param_len);

    wasm_function_inst_t func = NULL;
    func = wasm_runtime_lookup_function(sp_ppl_wasm_module_inst, "PPL_Initialize", NULL);
    if (NULL == func) {
        DBG_PRINTF("The PPL_Initialsm function is not found.");
        goto end;
    }

    argv[0] = network_id;
    argv[1] = wasm_buffer;
    // pass 2 elements for function arguments
    if (!wasm_runtime_call_wasm(sp_ppl_wasm_exec_env, func, argc, argv)) {
        DBG_PRINTF("call wasm function PPL_Initialize failed. %s", wasm_runtime_get_exception(sp_ppl_wasm_module_inst));
        goto end;
    }

    ret = (EPPL_RESULT_CODE)argv[0];
    DBG_PRINTF("Native finished calling wasm function: PPL_Initialize(), returned value: %d", ret);
    if (E_PPL_OK != ret) {
        DBG_PRINTF("PPL_Initialize() error: [%d]", (int)ret);
        goto end;
    }
    ret = E_PPL_OK;

end:
    if (0 != wasm_buffer) {
        wasm_runtime_module_free(sp_ppl_wasm_module_inst, wasm_buffer);
    }

    if(E_PPL_OK != ret){
        if (NULL != sp_ppl_wasm_exec_env) {
            wasm_runtime_destroy_exec_env(sp_ppl_wasm_exec_env);
            sp_ppl_wasm_exec_env = NULL;
        }
    }

    /* Always return E_PPL_OK. We don't force the user to use PPL wasm. */
    /* If failed to load PPL wasm, the system output OutputTensor directly. */
    ret = E_PPL_OK;

    return ret;
}

/*----------------------------------------------------------------------*/
EPPL_RESULT_CODE PPL_AnalyzeOutput(uint32_t p_data, uint32_t in_size, uint32_t pp_out_buf, uint32_t p_out_size, uint32_t p_upload_flag)
{
    DBG_PRINTF("PPL_AnalyzeOutput");
    DBG_PRINTF("p_data=(0x%x)", p_data);
    DBG_PRINTF("in_size=(%d)", in_size);
    DBG_PRINTF("pp_out_buf=(0x%x)", pp_out_buf);
    DBG_PRINTF("p_out_size=(0x%x)", p_out_size);
    DBG_PRINTF("p_upload_flag=(0x%x)", p_upload_flag);

    wasm_function_inst_t func_ppl_analyze = NULL;
    uint32_t argv[5] = {0};
    uint32_t argc = sizeof(argv) / sizeof(uint32_t);
    EPPL_RESULT_CODE rtn = E_PPL_OK;

    if (0 == p_data) {
        ERR_PRINTF("p_data is NULL");
        return E_PPL_OTHER;
    }
    if (0 == pp_out_buf) {
        ERR_PRINTF("pp_out_buf is NULL");
        return E_PPL_OTHER;
    }
    if (0 == p_out_size) {
        ERR_PRINTF("p_out_size is NULL");
        return E_PPL_OTHER;
    }
    if (0 == p_upload_flag) {
        ERR_PRINTF("p_upload_flag is NULL");
        return E_PPL_OTHER;
    }
    if (NULL == sp_ppl_wasm_module_inst) {
        ERR_PRINTF("sp_ppl_wasm_module_inst is NULL");
        return E_PPL_OTHER;
    }
    if (NULL == sp_ppl_wasm_exec_env) {
        ERR_PRINTF("sp_ppl_wasm_exec_env is NULL");
        return E_PPL_OTHER;
    }
    argv[0] = p_data;
    argv[1] = in_size;
    argv[2] = pp_out_buf;
    argv[3] = p_out_size;
    argv[4] = p_upload_flag;
    
    for (uint32_t i = 0; i < argc; i++) {
        DBG_PRINTF("argv[%d]=%u", i, argv[i]);
    }

    func_ppl_analyze = wasm_runtime_lookup_function(sp_ppl_wasm_module_inst, "PPL_Analyze", NULL);
    if (NULL == func_ppl_analyze){
        ERR_PRINTF("The wasm function PPL_Analyze wasm function is not found.");
        return E_PPL_OTHER;
    }
    DBG_PRINTF("lookup_function successed.");

    if (wasm_runtime_call_wasm(sp_ppl_wasm_exec_env, func_ppl_analyze, argc, argv) ) {
        rtn = (EPPL_RESULT_CODE)argv[0];
        DBG_PRINTF("Native finished calling wasm function: PPL_Analyze, returned: %d", rtn);
    }
    else {
        ERR_PRINTF("call wasm function PPL_Analyze failed. error: %s", wasm_runtime_get_exception(sp_ppl_wasm_module_inst));
        rtn = E_PPL_OTHER;
    }
    return rtn;
}

/*----------------------------------------------------------------------*/
EPPL_RESULT_CODE PPL_ResultRelease(uint32_t result)
{
    DBG_PRINTF("%s", __func__);
    DBG_PRINTF("result=(%u)",result);

    wasm_function_inst_t func_ppl_resultrelease = NULL;
    uint32_t argv[1] = {0};
    uint32_t argc = sizeof(argv) / sizeof(uint32_t);
    EPPL_RESULT_CODE rtn = E_PPL_OK;

    if (0 == result){
        ERR_PRINTF("result ERROR");
        return E_PPL_OTHER;
    }
    if (NULL == sp_ppl_wasm_module_inst) {
        ERR_PRINTF("sp_ppl_wasm_module_inst is NULL");
        return E_PPL_OTHER;
    }
    if (NULL == sp_ppl_wasm_exec_env) {
        ERR_PRINTF("sp_ppl_wasm_exec_env is NULL");
        return E_PPL_OTHER;
    }
    argv[0] = result;

    DBG_PRINTF("argv[0]=%u",argv[0]);

    func_ppl_resultrelease = wasm_runtime_lookup_function(sp_ppl_wasm_module_inst, "PPL_ResultRelease", NULL);
    if (NULL == func_ppl_resultrelease) {
        ERR_PRINTF("The wasm function PPL_ResultRelease wasm function is not found.");
        return E_PPL_OTHER;
    }

    if (wasm_runtime_call_wasm(sp_ppl_wasm_exec_env, func_ppl_resultrelease, argc, argv) ) {
        rtn = (EPPL_RESULT_CODE)argv[0];
        DBG_PRINTF("Native finished calling wasm function: PPL_ResultRelease, returned: %d", rtn);
    }
    else {
        ERR_PRINTF("call wasm function PPL_ResultRelease failed. error: %s", wasm_runtime_get_exception(sp_ppl_wasm_module_inst));
        rtn = E_PPL_OTHER;
    }
    return rtn;
}

/*----------------------------------------------------------------------*/
/* PPL_Finalize
 * return: Always return E_PPL_OK. We don't force the user to use PPL wasm.
 *         If failed to load PPL wasm, the system output OutputTensor directly.
 */
EPPL_RESULT_CODE PPL_Finalize(void)
{
    INF_PRINTF("%s", __func__);

    // Next we will pass a buffer to the WASM function
    wasm_function_inst_t func_ppl_finalize = NULL;
    uint32_t argv[1] = {0};

    if (NULL == sp_ppl_wasm_module_inst) {
        DBG_PRINTF("sp_ppl_wasm_module_inst is NULL");
        return E_PPL_OK;
    }
    if (NULL == sp_ppl_wasm_exec_env) {
        DBG_PRINTF("sp_ppl_wasm_exec_env is NULL");
        return E_PPL_OK;
    }

    func_ppl_finalize = wasm_runtime_lookup_function(sp_ppl_wasm_module_inst, "PPL_Finalize", NULL);
    if (NULL == func_ppl_finalize) {
        DBG_PRINTF("The wasm function PPL_Finalize wasm function is not found.");
        return E_PPL_OK;
    }

    if (wasm_runtime_call_wasm(sp_ppl_wasm_exec_env, func_ppl_finalize, 0, argv)) {
        DBG_PRINTF("Native finished calling wasm function: PPL_Finalize, returned: %d", *(uint32_t*)argv);
    }
    else {
        DBG_PRINTF("call wasm function PPL_Finalize failed. error: %s", wasm_runtime_get_exception(sp_ppl_wasm_module_inst));
    }

    if (sp_ppl_wasm_exec_env) {
        wasm_runtime_destroy_exec_env(sp_ppl_wasm_exec_env);
        sp_ppl_wasm_exec_env = NULL;
    }
    return E_PPL_OK;
}

const char* PPL_GetPplVersion(void)
{
    DBG_PRINTF("PPL_GetPplVersion");

    // Next we will pass a buffer to the WASM function
    wasm_function_inst_t func_ppl_getpplversion = NULL;
    uint32_t argv[1] = {0};
    char*  rtn = NULL;

    if (NULL == sp_ppl_wasm_module_inst) {
        ERR_PRINTF("sp_ppl_wasm_module_inst is NULL");
        return rtn;
    }
    if (NULL == sp_ppl_wasm_exec_env) {
        ERR_PRINTF("sp_ppl_wasm_exec_env is NULL");
        return rtn;
    }

    func_ppl_getpplversion = wasm_runtime_lookup_function(sp_ppl_wasm_module_inst, "PPL_GetPplVersion", NULL);
    if (NULL == func_ppl_getpplversion) {
        ERR_PRINTF("The wasm function PPL_GetPplVersion wasm function is not found.");
        return rtn;
    }

    if (wasm_runtime_call_wasm(sp_ppl_wasm_exec_env, func_ppl_getpplversion, 0, argv) ) {
        rtn = (char *)wasm_runtime_addr_app_to_native(sp_ppl_wasm_module_inst, *(uint32_t *)argv);
        DBG_PRINTF("Native finished calling wasm function: PPL_GetPplVersion, returned: %p", rtn);
    }
    else {
        ERR_PRINTF("call wasm function PPL_GetPplVersion failed. error: %s", wasm_runtime_get_exception(sp_ppl_wasm_module_inst));
        rtn = NULL;
    }
    return rtn;
}

uint32_t SetupWasm(wasm_module_inst_t inst, wasm_exec_env_t exec_env)
{
    sp_ppl_wasm_module_inst = inst;
    sp_ppl_wasm_exec_env = exec_env;
    return E_PPL_OK;
}

uint32_t SetupParam(const char *param, size_t size)
{
    s_ppl_paramater = param;
    s_ppl_paramater_size = size;
    return E_PPL_OK;
}
