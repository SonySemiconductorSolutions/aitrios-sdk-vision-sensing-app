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

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "wasm_export.h"
#include "bh_platform.h"
#include "bh_read_file.h"
#include "bh_getopt.h"
#if BH_HAS_DLFCN
#include <dlfcn.h>
#endif
// #include "network_struct.h"
#include "parson.h"

#include "ppl_if.h"
#include "loader.h"

const uint32_t ANALYZE_COUNT = 1;
const uint32_t ANALYZE_INTERVAL_SECONDS = 1;
const uint32_t DUMMY_NETWORK_ID = 0x00111111;

void print_usage(void) {
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -f [path of wasm file] \n");
    fprintf(stdout, "  -n [path of native library file] \n");
    fprintf(stdout, "  -g [(optional) debug option] \n");
    fprintf(stdout, "  -h [usage] \n");
    fprintf(stdout, "  -? [usage] \n");    
}

int main(int argc, char *argv_main[]) {
#if BH_HAS_DLFCN
    const char *native_lib_list[8] = { NULL };
    uint32 native_lib_count = 0;
    void *native_handle_list[8] = { NULL };
    // uint32 native_handle_count = 0;
#endif
    char *buffer = NULL;
    int opt;
    int ret_val = EXIT_FAILURE;
    int func_ret_val;
    char *wasm_path = NULL;

    bool is_debug = false;

    wasm_module_t module = NULL;
    wasm_module_inst_t module_inst = NULL;
    wasm_exec_env_t exec_env = NULL;

    while ((opt = getopt(argc, argv_main, "dhmf:t:n:o:p:g::")) != -1) {
        switch (opt) {
            case 'f':
                wasm_path = optarg; // wasm file path
                break;
            case 't':               // wasm type option(not used loader main) 
                break;
#if BH_HAS_DLFCN
            case 'n':
                printf("native_lib: %s\n", optarg);
                native_lib_list[native_lib_count++] = optarg;
                break;
#endif
            case 'o':               // output_tensor option(not used loader main)
                break;
            case 'p':               // ppl_parameter option(not used loader main)
               break;             
            case 'g':
                is_debug = true;
                break;
            case 'd':              // debug option(not used)
                break;
            case 'm':              // dump memory consumption(not used)
                break;
            case 'h':
                print_usage();
                return EXIT_SUCCESS;
            case '?':
                print_usage();
                return EXIT_SUCCESS;
        }
    }
    if (optind == 1) {
        print_usage();
        return EXIT_FAILURE;
    }

    // load wasm
    func_ret_val = Exec_Load(&wasm_path, &buffer, &module, &module_inst, &exec_env, is_debug,
                        native_lib_list, native_lib_count, native_handle_list);
    if (E_LOADER_OK != func_ret_val) {
        ret_val = EXIT_FAILURE;
        goto fail;
    }

    // call wasm
    struct SetupWasm_struct_t setup_wasm;
    setup_wasm.ppl_wasm_module_inst = module_inst;
    setup_wasm.ppl_wasm_exec_env = exec_env;

    printf("call PPL main\n");
    func_ret_val = PPL_main(&setup_wasm);
    printf("call PPL main result: %d\n", func_ret_val);

    // Output memory dump when Wasm exits
    wasm_runtime_dump_mem_consumption(exec_env);

    if (func_ret_val == 0) {
        ret_val = EXIT_SUCCESS;
    } else {
        ret_val = EXIT_FAILURE;
    }

fail:

    if (exec_env) {
        wasm_runtime_destroy_exec_env(exec_env);
    }
    if (module_inst) {
        wasm_runtime_deinstantiate(module_inst);
    }
    if (module) {
        wasm_runtime_unload(module);
    }
    if (buffer) {
        BH_FREE(buffer);
    }

    Exec_Unload(native_handle_list);

    wasm_runtime_destroy();

    return ret_val;
}
