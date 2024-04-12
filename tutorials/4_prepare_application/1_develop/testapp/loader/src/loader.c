// This library contains following modification to "WebAssembly Micro Runtime"
// - Use native libraries
// - Use debug instance
//
// Original source code is below
//   https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/product-mini/platforms/posix/main.c

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

/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdint.h>
#include "wasm_export.h"
#include "bh_platform.h"
#include "bh_read_file.h"
#include "bh_getopt.h"
#if BH_HAS_DLFCN
#include <dlfcn.h>
#endif

#include "loader.h"

#define HEAP_BUF_SIZE 8192 * 1024
const uint32 ERROR_BUF_SIZE = 128;
const uint32 WASM_BUF_SIZE = 4096 * 1024;
const uint32 WASM_STACK_SIZE = 32768;
const uint32 WASM_HEAP_SIZE = 4096 * 1024;
const char *WASM_DEBUG_IP_ADDR = "127.0.0.1";
const uint32 WASM_DEBUG_PORT = 1234;

static uint32 native_handle_count = 0;
// supporting native library
// based on https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/product-mini/platforms/posix/main.c
#if BH_HAS_DLFCN
typedef uint32 (*get_native_lib_func)(char **p_module_name,
                                      NativeSymbol **p_native_symbols);

static uint32
load_and_register_native_libs(const char **native_lib_list,
                              uint32 native_lib_count,
                              void **native_handle_list)
{
    // uint32 i, native_handle_count = 0, n_native_symbols;
    uint32 i = 0, n_native_symbols;
    NativeSymbol *native_symbols;
    char *module_name;
    void *handle;
    printf("native_lib_count: %d\n", native_lib_count);

    for (i = 0; i < native_lib_count; i++) {
        /* open the native library */
        if (!(handle = dlopen(native_lib_list[i], RTLD_NOW | RTLD_GLOBAL))
            && !(handle = dlopen(native_lib_list[i], RTLD_LAZY))) {
            LOG_WARNING("warning: failed to load native library %s",
                        native_lib_list[i]);
            continue;
        }

        /* lookup get_native_lib func */
        get_native_lib_func get_native_lib = dlsym(handle, "get_native_lib");
        if (!get_native_lib) {
            LOG_WARNING("warning: failed to lookup `get_native_lib` function "
                        "from native lib %s",
                        native_lib_list[i]);
            dlclose(handle);
            continue;
        }

        n_native_symbols = get_native_lib(&module_name, &native_symbols);

        /* register native symbols */
        if (!(n_native_symbols > 0 && module_name && native_symbols
              && wasm_runtime_register_natives(module_name, native_symbols,
                                               n_native_symbols))) {
            LOG_WARNING("warning: failed to register native lib %s",
                        native_lib_list[i]);
            dlclose(handle);
            continue;
        }

        native_handle_list[native_handle_count++] = handle;
    }

    printf("native_handle_count: %d\n", native_handle_count);
    return native_handle_count;
}

static void
unregister_and_unload_native_libs(uint32 native_lib_count,
                                  void **native_handle_list)
{
    uint32 i, n_native_symbols;
    NativeSymbol *native_symbols;
    char *module_name;
    void *handle;
    printf("native_lib_count: %d\n", native_lib_count);

    for (i = 0; i < native_lib_count; i++) {
        handle = native_handle_list[i];

        /* lookup get_native_lib func */
        get_native_lib_func get_native_lib = dlsym(handle, "get_native_lib");
        if (!get_native_lib) {
            LOG_WARNING("warning: failed to lookup `get_native_lib` function "
                        "from native lib %p",
                        handle);
            continue;
        }

        n_native_symbols = get_native_lib(&module_name, &native_symbols);
        if (n_native_symbols == 0 || module_name == NULL
            || native_symbols == NULL) {
            LOG_WARNING("warning: get_native_lib returned different values for "
                        "native lib %p",
                        handle);
            continue;
        }

        /* unregister native symbols */
        if (!wasm_runtime_unregister_natives(module_name, native_symbols)) {
            LOG_WARNING("warning: failed to unregister native lib %p", handle);
            continue;
        }

        dlclose(handle);
    }
}
#endif /* BH_HAS_DLFCN */

uint32_t Exec_Load(char **wasm_path, char **buffer, wasm_module_t *module, wasm_module_inst_t *module_inst, wasm_exec_env_t *exec_env, bool is_debug,
                   const char **native_lib_list,
                   uint32 native_lib_count,
                   void **native_handle_list) {

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

    printf("Exec_Load wasm_runtime_full_init start\n");

    res_init = wasm_runtime_full_init(&init_args);
    if (0 == res_init) {
        printf("Init runtime environment failed.\n");
        return E_LOADER_FAIL;
    }

    printf("Exec_Load load_and_register_native_libs start\n");


#if BH_HAS_DLFCN
    // native_handle_count = load_and_register_native_libs(
    //     native_lib_list, native_lib_count, native_handle_list);
    load_and_register_native_libs(
        native_lib_list, native_lib_count, native_handle_list);
#endif

    printf("Exec_Load bh_read_file_to_buffer start\n");

    *buffer = bh_read_file_to_buffer(*wasm_path, &buf_size);
    if (NULL == *buffer) {
        printf("Open wasm app file [%s] failed.\n", *wasm_path);
        return E_LOADER_FAIL;
    }

    printf("Exec_Load wasm_runtime_load start\n");

    *module = wasm_runtime_load(*buffer, buf_size, error_buf, sizeof(error_buf));
    if (NULL == *module) {
        printf("Load wasm module failed. error: %s\n", error_buf);
        return E_LOADER_FAIL;
    }

    printf("Exec_Load wasm_runtime_instantiate start\n");

    *module_inst = wasm_runtime_instantiate(*module, stack_size, heap_size,
                                           error_buf, sizeof(error_buf));
    if (NULL == *module_inst) {
        printf("Instantiate wasm module failed. error: %s\n", error_buf);
        return E_LOADER_FAIL;
    }

    printf("Exec_Load wasm_runtime_create_exec_env start\n");

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

void Exec_Unload(void **native_handle_list)
{
#if BH_HAS_DLFCN
    /* unload the native libraries */
    unregister_and_unload_native_libs(native_handle_count, native_handle_list);
#endif
}
