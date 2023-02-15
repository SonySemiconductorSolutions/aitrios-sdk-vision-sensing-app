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

#ifndef _LOADER_H_
#define _LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "wasm_export.h"

typedef enum {
    E_LOADER_OK,
    E_LOADER_FAIL
} LOADER_RESULT_CODE;

uint32_t Exec_Load(char **wasm_path, char **buffer, wasm_module_t *module, wasm_module_inst_t *module_inst, wasm_exec_env_t *exec_env, bool is_debug);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _LOADER_H_
