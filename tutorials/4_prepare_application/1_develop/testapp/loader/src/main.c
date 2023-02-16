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
#include <stdlib.h>
#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"
#include "network_struct.h"
#include "parson.h"

#include "ppl_if.h"
#include "loader.h"
#include "analyzer.h"

const uint32_t ANALYZE_COUNT = 1;
const uint32_t ANALYZE_INTERVAL_SECONDS = 1;
const uint32_t DUMMY_NETWORK_ID = 0x00111111;

const char *DEFAULT_IC_PPL_PARAMETER_FILE = "classification/ppl_parameter.json";
const char *DEFAULT_OD_PPL_PARAMETER_FILE = "objectdetection/ppl_parameter.json";
const char *DEFAULT_IC_OUTPUT_TENSOR_FILE = "classification/output_tensor.jsonc";
const char *DEFAULT_OD_OUTPUT_TENSOR_FILE = "objectdetection/output_tensor.jsonc";

void print_usage(void) {
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -f [path of wasm file] \n");
}

int main(int argc, char *argv_main[]) {
    char *buffer;
    int opt;
    int ret_val;
    char *wasm_path = NULL;
    char *wasm_type = "od";
    char *tensor_file_path = NULL;
    char *ppl_para_file_path = NULL;

    bool is_debug = false;

    wasm_module_t module = NULL;
    wasm_module_inst_t module_inst = NULL;
    wasm_exec_env_t exec_env = NULL;

    FILE *fp;
    int file_size, read_size;
    char *p_param;
    size_t p_param_size;
    JSON_Value *p_param_value;
    char *o_tensor;
    JSON_Value *o_tensor_value;
    JSON_Array *o_tensor_array;
    size_t o_tensor_data_length;
    float *mp_data;

    while ((opt = getopt(argc, argv_main, "dhf:t:o:p:g::")) != -1) {
        switch (opt) {
            case 'f':
                wasm_path = optarg; // wasm file path
                break;
            case 't':
                wasm_type = optarg; // "od" or "ic"
                break;
            case 'o':
                tensor_file_path = optarg; // output tensor file path
                break;
            case 'p':
                ppl_para_file_path = optarg; // ppl command parameters file path
                break;
            case 'g':
                is_debug = true;
                break;
            case 'd':              // debug option(not used)
                break;
            case 'h':
                print_usage();
                return 0;
            case '?':
                print_usage();
                return 0;
        }
    }
    if (optind == 1) {
        print_usage();
        return 0;
    }

    // Load ppl command parameter json

    printf("wasm_type: %s\n", wasm_type);
    bool isClassification = false;
    if (strlen(wasm_type) == 2 && strncmp(wasm_type, "ic", 2) == 0) {
        isClassification = true;
    }

    printf("isClassification: %d\n", isClassification);

    if (ppl_para_file_path != NULL) {
        fp = fopen(ppl_para_file_path, "r");
    } else {
        if (isClassification) {
            fp = fopen(DEFAULT_IC_PPL_PARAMETER_FILE, "r");
        } else {
            fp = fopen(DEFAULT_OD_PPL_PARAMETER_FILE, "r");
        }
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    p_param_size = sizeof(char) * (file_size + 1);
    p_param = (char *)malloc(p_param_size);
    read_size = fread(p_param, sizeof(char), file_size, fp);
    p_param[file_size] = '\0';
    if (file_size != read_size) {
        free(p_param);
        p_param = NULL;
    }
    fclose(fp);

    p_param_value = json_parse_string(p_param);
    if (json_value_get_type(p_param_value) != JSONObject) {
        printf("ppl command parameters json parse error. please check json file.\n");
        goto fail;
    } else {
        printf("ppl command parameters json parse ok.\n");
    }

    // Load output tensor json

    if (tensor_file_path != NULL) {
        fp = fopen(tensor_file_path, "r");
    } else {
        if (isClassification) {
            fp = fopen(DEFAULT_IC_OUTPUT_TENSOR_FILE, "r");
        } else {
            fp = fopen(DEFAULT_OD_OUTPUT_TENSOR_FILE, "r");
        }
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    o_tensor = (char *)malloc(sizeof(char) * (file_size + 1));
    read_size = fread(o_tensor, sizeof(char), file_size, fp);
    o_tensor[file_size] = '\0';
    if (file_size != read_size) {
        free(o_tensor);
        o_tensor = NULL;
    }
    fclose(fp);

    o_tensor_value = json_parse_string_with_comments(o_tensor);
    if (json_value_get_type(o_tensor_value) != JSONArray) {
        printf("output tensor json parse error. please check json file.\n");
        goto fail;
    } else {
        printf("output tensor json parse ok.\n");
    }
    o_tensor_array = json_value_get_array(o_tensor_value);
    o_tensor_data_length = json_array_get_count(o_tensor_array);
    if (o_tensor_data_length == 0) {
        goto fail;
    }
    mp_data = (float *)malloc(sizeof(float) * o_tensor_data_length);
    for (size_t i = 0; i < o_tensor_data_length; i++) {
        double o_tensor_data = json_array_get_number(o_tensor_array, i);
        mp_data[i] = o_tensor_data;
    }

    // load wasm

    ret_val = Exec_Load(&wasm_path, &buffer, &module, &module_inst, &exec_env, is_debug);
    if (E_LOADER_OK != ret_val) {
        goto fail;
    }

    SetupWasm(module_inst, exec_env);
    SetupParam(p_param, p_param_size);

    // call wasm

    uint32_t network_id = DUMMY_NETWORK_ID;
    ret_val = PPL_Initialize(network_id);
    if (E_PPL_OK != ret_val) {
        goto fail;
    }

    const char *str = NULL;
    str = PPL_GetPplVersion();
    if (str) {
        printf("PPL_GetPplVersion result: %s\n", str);
    } else {
        goto fail;
    }

    for (uint32_t i = 0; i < ANALYZE_COUNT; i++) {
        size_t header_size = sizeof(Sc_ByebufferHeader);
        size_t o_tensor_size = sizeof(float) * o_tensor_data_length;
        uint8_t *o_tensor_buffer = malloc(header_size + o_tensor_size);
        Sc_ByebufferHeader *img_addr = (Sc_ByebufferHeader *)o_tensor_buffer;
        img_addr->image_size = o_tensor_size;
        img_addr->header_size = header_size;
        memcpy(o_tensor_buffer + header_size, mp_data, o_tensor_size);

        Exec_PPL_Analyze(module_inst, img_addr);

        free(o_tensor_buffer);
        o_tensor_buffer = NULL;

        sleep(ANALYZE_INTERVAL_SECONDS);
    }

    ret_val = PPL_Finalize();
    if (E_PPL_OK != ret_val) {
        goto fail;
    }

fail:
    json_value_free(o_tensor_value);
    if (o_tensor) {
        free(o_tensor);
        o_tensor = NULL;
    }

    json_value_free(p_param_value);
    if (p_param) {
        free(p_param);
        p_param = NULL;
    }

    if (mp_data) {
        free(mp_data);
        mp_data = NULL;
    }

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
    wasm_runtime_destroy();
    return 0;
}
