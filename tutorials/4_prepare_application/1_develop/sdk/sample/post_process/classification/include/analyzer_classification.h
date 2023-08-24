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

#include "parson.h"

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define PPL_ID_VERSION "00.01.01.00"            // Format: "AA.XX.YY.ZZ" where AA: ID, XX.YY.ZZ: Version
#define PPL_DNN_OUTPUT_CLASSES           (18)
#define PPL_DEFAULT_MAX_PREDICTIONS      (5)

/* enum */
typedef enum {
    E_PPL_OK,
    E_PPL_INVALID_PARAM,
    E_PPL_E_MEMORY_ERROR,
    E_PPL_INVALID_STATE,
    E_PPL_OTHER
} EPPL_RESULT_CODE;

typedef struct tagPPL_ClsParam{
    uint16_t dnnOutputClasses;
    uint16_t maxPredictions;
} PPL_ClsParam;

EPPL_RESULT_CODE PPL_ClassificationAnalyze(float *p_data, uint32_t in_size, void **pp_out_buf, uint32_t *p_out_size, PPL_ClsParam cls_param);
EPPL_RESULT_CODE json_parse(JSON_Value *root_value, PPL_ClsParam *p_cls_param);
