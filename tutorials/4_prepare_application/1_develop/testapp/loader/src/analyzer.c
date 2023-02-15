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

#include <stdlib.h>
#include <stdint.h>
#include "network_struct.h"

#include "analyzer.h"

#include "ppl_if.h"

#define ERR_PRINTF(fmt, ...)
#define INF_PRINTF(fmt, ...)
#define DBG_PRINTF(fmt, ...)

int Exec_PPL_Analyze(wasm_module_inst_t  module_inst,
                     Sc_ByebufferHeader *output) {
    int ret = -1;
    EPPL_RESULT_CODE ppl_result = E_PPL_OK;
    PPL_AnalyzeOutput_argv_t argv = {0};
    void *wasm_p_out_buf = NULL;
    void *native_p_out_buf = NULL;
    uint32_t out_size = 0;
    bool upload_flag = false;

    if (INT_MAX < output->image_size) {
        ERR_PRINTF("output->image_size error: [%d]", output->image_size);
        goto end;
    }

    argv.wasm.pdata = wasm_runtime_module_malloc(module_inst, output->image_size, (void**)&argv.native.pdata);
    if ((0 == argv.wasm.pdata) || (NULL == argv.native.pdata)) {
        ERR_PRINTF("wasm_runtime_module_malloc ERROR");
        goto end;
    }
    memcpy(argv.native.pdata, (uint8_t *)output + output->header_size, output->image_size);

    argv.wasm.in_size = output->image_size / sizeof(float);

    argv.wasm.pp_out_buf = wasm_runtime_module_malloc(module_inst, sizeof(void*), NULL);
    if (0 == argv.wasm.pp_out_buf) {
        ERR_PRINTF("wasm_runtime_module_malloc ERROR");
        goto end;
    }

    argv.wasm.p_out_size = wasm_runtime_module_malloc(module_inst, sizeof(uint32_t), NULL);
    if (0 == argv.wasm.p_out_size) {
        ERR_PRINTF("wasm_runtime_module_malloc ERROR");
        goto end;
    }

    argv.wasm.p_upload_flag = wasm_runtime_module_malloc(module_inst, sizeof(bool), NULL);
    if (0 == argv.wasm.p_upload_flag) {
        ERR_PRINTF("wasm_runtime_module_malloc ERROR");
        goto end;
    }

    ppl_result = PPL_AnalyzeOutput(argv.wasm.pdata, argv.wasm.in_size,
                                   argv.wasm.pp_out_buf, argv.wasm.p_out_size,
                                   argv.wasm.p_upload_flag);
    if (E_PPL_OK != ppl_result) {
        ERR_PRINTF("PPL_Analyze failed. [%d]", ppl_result);
        goto end;
    }

    INF_PRINTF("PPL_Analyze successed.");

    argv.native.pp_out_buf = (void**)wasm_runtime_addr_app_to_native(module_inst, argv.wasm.pp_out_buf);
    if (NULL == argv.native.pp_out_buf) {
        ERR_PRINTF("wasm_runtime_addr_app_to_native ERROR.(pp_out_buf is NULL)");
        goto end;
    }
    DBG_PRINTF("pp_out_buf:%p", argv.native.pp_out_buf);
    wasm_p_out_buf = *argv.native.pp_out_buf;
    DBG_PRINTF("wasm_p_out_buf:%p", wasm_p_out_buf);
    native_p_out_buf = wasm_runtime_addr_app_to_native(module_inst, (uint32_t)wasm_p_out_buf);
    if (NULL == native_p_out_buf) {
        ERR_PRINTF("wasm_runtime_addr_app_to_native ERROR.(output is NULL)");
        goto end;
    }
    DBG_PRINTF("native_p_out_buf:%p", native_p_out_buf);

    argv.native.p_out_size = (uint32_t*)wasm_runtime_addr_app_to_native(module_inst, argv.wasm.p_out_size);
    if (NULL == argv.native.p_out_size) {
        ERR_PRINTF("wasm_runtime_addr_app_to_native ERROR.(p_out_size is NULL)");
        goto end;
    }
    out_size = *argv.native.p_out_size;
    DBG_PRINTF("out_size:%u", out_size);

    argv.native.p_upload_flag = (bool*)wasm_runtime_addr_app_to_native(module_inst, argv.wasm.p_upload_flag);
    if (NULL == argv.native.p_upload_flag) {
        ERR_PRINTF("wasm_runtime_addr_app_to_native ERROR.(p_upload_flag is NULL)");
        goto end;
    }
    upload_flag = *argv.native.p_upload_flag;
    DBG_PRINTF("upload_flag:%s", (upload_flag ? "true" : "false"));

    if (false == upload_flag) {
        INF_PRINTF("PPL_Analyze p_upload_flag is false.");
        ret = -2;
        goto end;
    }

    // NormalizationToJSON(NULL, native_p_out_buf, out_size, output->filename);
    ret = 0;

end:

   if (NULL != wasm_p_out_buf) {
        ppl_result = PPL_ResultRelease((uint32_t)wasm_p_out_buf);
        if (E_PPL_OK != ppl_result) {
            ERR_PRINTF("PPL_ResultRelease failed. [%d]", ppl_result);
        }
    }
    if(0 != argv.wasm.pdata) {
        wasm_runtime_module_free(module_inst, argv.wasm.pdata);
    }
    if(0 != argv.wasm.pp_out_buf) {
        wasm_runtime_module_free(module_inst, argv.wasm.pp_out_buf);
    }
    if(0 != argv.wasm.p_out_size) {
        wasm_runtime_module_free(module_inst, argv.wasm.p_out_size);
    }
    if(0 != argv.wasm.p_upload_flag) {
        wasm_runtime_module_free(module_inst, argv.wasm.p_upload_flag);
    }
    return ret;
}
