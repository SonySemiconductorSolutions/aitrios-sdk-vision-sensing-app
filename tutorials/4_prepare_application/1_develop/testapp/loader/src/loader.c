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

#include <stdint.h>
#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"

#include "loader.h"

#define HEAP_BUF_SIZE 1024 * 1024
const uint32 ERROR_BUF_SIZE = 128;
const uint32 WASM_BUF_SIZE = 1024 * 1024;
const uint32 WASM_STACK_SIZE = 8192;
const uint32 WASM_HEAP_SIZE = 1024 * 1024;
const char *WASM_DEBUG_IP_ADDR = "127.0.0.1";
const uint32 WASM_DEBUG_PORT = 1234;

uint32_t Exec_Load(char **wasm_path, char **buffer, wasm_module_t *module, wasm_module_inst_t *module_inst, wasm_exec_env_t *exec_env, bool is_debug) {

    static char global_heap_buf[HEAP_BUF_SIZE];

    char error_buf[ERROR_BUF_SIZE];
    uint32 buf_size = WASM_BUF_SIZE, stack_size = WASM_STACK_SIZE, heap_size = WASM_HEAP_SIZE;
    RuntimeInitArgs init_args;

    int res_init;

    memset(&init_args, 0, sizeof(RuntimeInitArgs));
    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    if (is_debug) {
        // refs: https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/source_debugging.md#enable-debugging-in-embedders-for-interpreter
        strncpy(init_args.ip_addr, WASM_DEBUG_IP_ADDR, sizeof(init_args.ip_addr) - 1);
        init_args.instance_port = WASM_DEBUG_PORT;
    }

    res_init = wasm_runtime_full_init(&init_args);
    if (0 == res_init) {
        printf("Init runtime environment failed.\n");
        return E_LOADER_FAIL;
    }

    *buffer = bh_read_file_to_buffer(*wasm_path, &buf_size);
    if (NULL == *buffer) {
        printf("Open wasm app file [%s] failed.\n", *wasm_path);
        return E_LOADER_FAIL;
    }

    *module = wasm_runtime_load(*buffer, buf_size, error_buf, sizeof(error_buf));
    if (NULL == *module) {
        printf("Load wasm module failed. error: %s\n", error_buf);
        return E_LOADER_FAIL;
    }

    *module_inst = wasm_runtime_instantiate(*module, stack_size, heap_size,
                                           error_buf, sizeof(error_buf));
    if (NULL == *module_inst) {
        printf("Instantiate wasm module failed. error: %s\n", error_buf);
        return E_LOADER_FAIL;
    }

    *exec_env = wasm_runtime_create_exec_env(*module_inst, stack_size);
    if (NULL == *exec_env) {
        printf("Create wasm execution environment failed.\n");
        return E_LOADER_FAIL;
    }

    if (is_debug) {
        // refs: https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/product-mini/platforms/posix/main.c
        uint32_t debug_port = wasm_runtime_start_debug_instance(*exec_env);
        if (0 == debug_port) {
            printf("Failed to start debug instance\n");
        }
        printf("Debug port : %d\n", debug_port);
    }

    return E_LOADER_OK;
}
