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

#ifndef _PPL_PUBLIC_H_
#define _PPL_PUBLIC_H_
#define EXPORT_API __attribute__((visibility("default")))

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
/* enum */
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


/* -------------------------------------------------------- */
/* API function                                             */
/* -------------------------------------------------------- */
/**
 * Initializes the PPL for selected network
 * 
 * @param network_id network ID
 * @param p_param Pointer to json string containing PPL specific parameters
 * @return Success or failure EPPL_RESULT_CODE
 */
EXPORT_API EPPL_RESULT_CODE PPL_Initialize(uint32_t network_id, const char *p_param);
/**
 * Post processing function
 * 
 * @param p_data Structure with information about PPL output
 * @param in_size Size of PPL input (number of float elements)
 * @param pp_out_buf Pointer to memory address where function shall store address of flat buffer
 * @param p_out_size Pointer to uint32_t where function shall store buffer size of serialized flat buffer memory
 * @param p_upload_flag Pointer to flag to specify whether to upload PPL Output to cloud or not.
 * @return Success or failure EPPL_RESULT_CODE
 */
EXPORT_API EPPL_RESULT_CODE PPL_Analyze(float *p_data, uint32_t in_size, void **pp_out_buf, uint32_t *p_out_size, bool *p_upload_flag);
/**
 * To release memory used for analysis
 * 
 * @param p_result Pointer to memory to be released after PPL_Analyse()
 * @return Success or failure EPPL_RESULT_CODE
 */
EXPORT_API EPPL_RESULT_CODE PPL_ResultRelease(void *p_result);
/**
 * Finalizes the PPL
 * 
 * @return Success or failure EPPL_RESULT_CODE
 */
EXPORT_API EPPL_RESULT_CODE PPL_Finalize(void);
/**
 * API to get PPL version
 * 
 * @return const char* indicating PPL Version
 */
EXPORT_API const char* PPL_GetPplVersion(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _PPL_PUBLIC_H_ */
