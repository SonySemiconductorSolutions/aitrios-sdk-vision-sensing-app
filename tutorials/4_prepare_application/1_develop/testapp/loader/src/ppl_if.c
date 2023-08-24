/****************************************************************************
 * Copyright 2022-2023 Sony Semiconductor Solutions Corp. All rights reserved.
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
// #include "network_struct.h"
#include "wasm_export.h"

/* Pivate definitions */

// #define ERR_PRINTF(fmt, ...)  DLog(Log_Error, "ScPP", fmt, ##__VA_ARGS__)
// #define INF_PRINTF(fmt, ...)  DLog(Log_Info,  "ScPP", fmt, ##__VA_ARGS__)
// #define DBG_PRINTF(fmt, ...)  DLog(Log_Debug, "ScPP", fmt, ##__VA_ARGS__)
#define ERR_PRINTF(fmt, ...)
#define INF_PRINTF(fmt, ...)
#define DBG_PRINTF(fmt, ...)

int PPL_main(struct SetupWasm_struct_t *setup_wasm)
{
    // Next we will pass a buffer to the WASM function
    wasm_function_inst_t func_ppl_main = NULL;
    // uint32_t argv[1] = {0};
    int rtn = 1;

    if (NULL == setup_wasm->ppl_wasm_module_inst) {
        printf("sp_ppl_wasm_module_inst is NULL\n");

        return rtn;
    }
    if (NULL == setup_wasm->ppl_wasm_exec_env) {
        printf("sp_ppl_wasm_exec_env is NULL\n");
        return rtn;
    }

    func_ppl_main = wasm_runtime_lookup_wasi_start_function(setup_wasm->ppl_wasm_module_inst);
    if (NULL == func_ppl_main) {
        printf("The main wasm function is not found.\n");
        return rtn;
    }

    if (!wasm_runtime_call_wasm(setup_wasm->ppl_wasm_exec_env, func_ppl_main, 0, NULL)) {
        printf("call wasm function main failed. error: %s\n", wasm_runtime_get_exception(setup_wasm->ppl_wasm_module_inst));
    } else {
        printf("call wasm function main succeeded.\n");

        rtn = 0;
    }

    return rtn;
}