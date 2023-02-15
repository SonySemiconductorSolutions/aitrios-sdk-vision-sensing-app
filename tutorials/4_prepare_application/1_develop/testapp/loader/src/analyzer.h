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

#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "wasm_export.h"

/* Streaming information */

typedef enum
{
    Sc_NoInfo = 0,         // No information
    Sc_Term,               // Streaming termination
    Sc_StreamingFrameInfoMax
}
Sc_StreamingFrameInfo;

/* Bytebuffer header */

typedef struct
{
    uint32_t              header_size;   // This header size(Include header_size)
    uint32_t              image_size;    // InputTensor size
    uint32_t              network_id;    // NetworkID
    Sc_StreamingFrameInfo info;          // Streaming information
    uint32_t              usr_data;      // User parameter
    int32_t               frame_cnt;     // Total number of frames to notify (Used internally for SensorControl)
    int32_t               frame_num;     // Number of frame notify           (Used internally for SensorControl)
    uint32_t              filename_len;  // File name string length
    char                  filename[0];   // Failname
}
Sc_ByebufferHeader;

int Exec_PPL_Analyze(wasm_module_inst_t  module_inst,
                     Sc_ByebufferHeader *output);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _ANALYZER_H_
