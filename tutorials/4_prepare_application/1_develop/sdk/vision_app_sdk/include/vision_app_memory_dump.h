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
#ifndef TESTAPP_DUMP_MEMORY_CONSUMPTION
#error "Enable TESTAPP_DUMP_MEMORY_CONSUMPTION."
#endif

#ifndef _VISION_APP_MEMORY_DUMP_H_
#define _VISION_APP_MEMORY_DUMP_H_
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Output Wasm execution code, dynamic memory, etc memory consumption to TERMINAL.
*   Using wasm_runtime_dump_mem_consumption().
*
*/
void TESTAPP_dump_mem_consumption();

#ifdef __cplusplus
}
#endif

#endif
