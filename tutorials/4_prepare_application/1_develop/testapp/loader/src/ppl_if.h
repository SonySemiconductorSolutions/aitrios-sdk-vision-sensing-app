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

#ifndef _PPL_IF_H_
#define _PPL_IF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "wasm_export.h"

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define PPL_STACK_SIZE (8192)

/* -------------------------------------------------------- */
/* enum                                                     */
/* -------------------------------------------------------- */
typedef enum {
    E_PPL_OK,
    E_PPL_INVALID_PARAM,
    E_PPL_E_MEMORY_ERROR,
    E_PPL_INVALID_STATE,
    E_PPL_OTHER
} EPPL_RESULT_CODE;

/* -------------------------------------------------------- */
/* structure                                                */
/* -------------------------------------------------------- */
/* common */
typedef struct {
    float      *pdata;         /* in */
    /*uint32_t in_size;*/      /* in */
    void       **pp_out_buf;   /* out */
    uint32_t   *p_out_size;    /* out */
    bool       *p_upload_flag; /* out */
} PPL_AnalyzeOutput_native_t;
typedef struct {
    uint32_t pdata;         /* in */
    uint32_t in_size;       /* in */
    uint32_t pp_out_buf;    /* out */
    uint32_t p_out_size;    /* out */
    uint32_t p_upload_flag; /* out */
} PPL_AnalyzeOutput_wasm_t;
typedef struct {
    PPL_AnalyzeOutput_native_t native;
    PPL_AnalyzeOutput_wasm_t wasm;
} PPL_AnalyzeOutput_argv_t;

/* wasm interface*/
struct SetupWasm_struct_t{
    wasm_module_inst_t ppl_wasm_module_inst;
    wasm_exec_env_t ppl_wasm_exec_env;
};
/* -------------------------------------------------------- */
/* API function                                             */
/* -------------------------------------------------------- */

int PPL_main(struct SetupWasm_struct_t *setup_wasm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _PPL_IF_H_
