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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

#include "vision_app_public.h"
#include "analyzer_semanticsegmentation.h"
#include "semantic_segmentation_generated.h"

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define PPL_ERR_PRINTF(fmt, ...) fprintf(stderr, "E [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n")
#define PPL_WARN_PRINTF(fmt, ...) fprintf(stderr, "W [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n")
#define PPL_INFO_PRINTF(fmt, ...) fprintf(stdout, "I [VisionAPP] ");fprintf(stdout, fmt, ##__VA_ARGS__);fprintf(stdout, "\n")
#define PPL_DBG_PRINTF(fmt, ...) printf( "D [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n")
#define PPL_VER_PRINTF(fmt, ...) printf( "V [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n")

/* -------------------------------------------------------- */
/* structure                                                */
/* -------------------------------------------------------- */
typedef struct tagSegmentationOutputTensor {
    std::vector<std::vector<float>> classes;
} SegmentationOutputTensor;

typedef struct tagSegmentationData {
    uint16_t height;
    uint16_t width;
    std::vector<std::vector<uint16_t>> class_id_map;
    uint16_t num_class_id;
    std::vector<std::vector<std::vector<float>>> score_map;
} SegmentationData;

/* -------------------------------------------------------- */
/* static                                                   */
/* -------------------------------------------------------- */
static EPPL_RESULT_CODE PPL_SegmentationParamInit(JSON_Value *root_value, PPL_SegmentationParam* p_segmentation_param);
static void createSegmentationData(float* data_body, uint32_t data_size, SegmentationOutputTensor *segmentaion_output);
static void analyzeSegmentationOutput(SegmentationOutputTensor out_tensor, SegmentationData *output_segmentation_data);
static void createSegmentationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const SegmentationData *segmentationData);

/**
 * @brief PPL_SegmentationAnalyze
 */
EPPL_RESULT_CODE PPL_SegmentationAnalyze(float *p_data, uint32_t in_size, void **pp_out_buf,  uint32_t *p_out_size, PPL_SegmentationParam segmentation_param) {

    uint8_t *p_out_param = NULL;
    SegmentationOutputTensor segmentation_output;
    SegmentationData output_segmentation_data;

    PPL_DBG_PRINTF("PPL_SegmentationAnalyze");

    if (p_data == NULL) {
        PPL_ERR_PRINTF("PPL:Invalid param : pdata=NULL");
        return E_PPL_INVALID_PARAM;
    }

    if (in_size != segmentation_param.dnnOutputTensorSize) {
        PPL_ERR_PRINTF("PPL:Unexpected value for : pdata->m_data_num %d",in_size);
        PPL_ERR_PRINTF("PPL:Expected value for : in_size %d",(uint32_t)(segmentation_param.dnnOutputTensorSize));
        return E_PPL_INVALID_PARAM;
    }

    /* call interface process */
    createSegmentationData(p_data, segmentation_param.dnnOutputTensorSize, &segmentation_output);

    /* call analyze process */
    analyzeSegmentationOutput(segmentation_output, &output_segmentation_data);

    /* serialize output data to flatbuffer */
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    createSegmentationFlatbuffer(builder, &output_segmentation_data);

    uint8_t* buf_ptr = builder->GetBufferPointer();
    if (buf_ptr == NULL) {
        PPL_ERR_PRINTF("Error could not create Flatbuffer");
        builder->Clear();
        delete(builder);
        return E_PPL_OTHER;
    }
    uint32_t buf_size = builder->GetSize();

    p_out_param = (uint8_t*)SessMalloc(buf_size);
    if (p_out_param == NULL) {
        PPL_ERR_PRINTF("malloc failed for creating flatbuffer, malloc size=%d", buf_size);
        builder->Clear();
        delete(builder);
        return E_PPL_E_MEMORY_ERROR;
    }

    /* Copy Data */
    memcpy(p_out_param, buf_ptr,buf_size);
    *pp_out_buf = p_out_param;
    *p_out_size = buf_size;

    //Clean up
    builder->Clear();
    delete(builder);

    return E_PPL_OK;
}


/**
 * json_parse: get json data from json_object and update in local struct
 * 
 * @param param Pointer to json string containing PPL specific parameters
 * @return Success or failure EPPL_RESULT_CODE
 */
EPPL_RESULT_CODE json_parse(JSON_Value *root_value, PPL_SegmentationParam* p_segmentation_param) {
    const char *p = NULL;
    const char* ppl_id_version = PPL_ID_VERSION;
    PPL_DBG_PRINTF("PPL_Initialize: <json_parse>");

    if (json_object_has_value(json_object(root_value),"header")) {
        if (json_object_has_value_of_type(json_object(root_value),"header",JSONObject)) {
            JSON_Object *header = json_object_get_object(json_object(root_value), "header");
            if (json_object_has_value(header,"id")) {
                p = json_object_get_string(header, "id");
                if (p != NULL) { 
                    if (strncmp(ppl_id_version, p, 2)== 0) {
                        PPL_DBG_PRINTF("[PPL_PUBLIC] header_id = %s",p);
                    } else {
                        PPL_DBG_PRINTF("[PPL_PUBLIC] header_id = %s",p);
                        return E_PPL_INVALID_PARAM;
                    }
                } else {
                    /* If header_id is NULL, use default PPL id */
                    PPL_DBG_PRINTF("[PPL_PUBLIC] ppl_id_version=%s", ppl_id_version);
                }
            } else {
                /* If json file does not have header_id, use default PPL id */
                PPL_DBG_PRINTF("[PPL_PUBLIC] ppl_id_version=%s", ppl_id_version);
            }

            if (json_object_has_value(header,"version")) {
                p = json_object_get_string(header, "version");
                if (p != NULL) {
                    if (strncmp(&ppl_id_version[3], p, 8) == 0) {
                        PPL_DBG_PRINTF("[PPL_PUBLIC] header_version p = %s ppl_id_version=%s",p,&ppl_id_version[3]);
                    } else {
                        PPL_DBG_PRINTF("[PPL_PUBLIC] header_version = %s",p);
                        return E_PPL_INVALID_PARAM;
                    }
                } else {
                    /* If header_version is NULL, use default PPL version */
                    PPL_DBG_PRINTF("[PPL_PUBLIC] ppl_id_version=%s", &ppl_id_version[3]);
                }
            } else {
                /* If json file does not have header_version, use default PPL version */
                PPL_DBG_PRINTF("[PPL_PUBLIC] ppl_id_version=%s", &ppl_id_version[3]);
            }
        }
    }
    return PPL_SegmentationParamInit(root_value, p_segmentation_param);
}


/* -------------------------------------------------------- */
/* private function                                         */
/* -------------------------------------------------------- */
static EPPL_RESULT_CODE PPL_SegmentationParamInit(JSON_Value *root_value,PPL_SegmentationParam* p_segmentation_param)
{
    // (void)root_value;
    PPL_DBG_PRINTF("PPL_SegmentationParamInit");   
    //TODO InputTensor size will enter
    // }

    if (json_object_has_value(json_object(root_value),"input_width")) {
        uint16_t input_width = json_object_get_number(json_object(root_value), "input_width");
        PPL_DBG_PRINTF("PPL_SemanticSegmentationParamInit input_width: %d", input_width);
        p_segmentation_param->inputWidth = input_width;
    } else {
        PPL_ERR_PRINTF("PPL_SemanticSegmentationParamInit: json file does not have input_width");
        return E_PPL_INVALID_PARAM;
    }

    if (json_object_has_value(json_object(root_value),"input_height")) {
        uint16_t input_height = json_object_get_number(json_object(root_value), "input_height");
        PPL_DBG_PRINTF("PPL_SemanticSegmentationParamInit input_height: %d", input_height);
        p_segmentation_param->inputHeight = input_height;
    } else {
        PPL_ERR_PRINTF("PPL_SemanticSegmentationParamInit: json file does not have input_height");
        return E_PPL_INVALID_PARAM;
    }

    p_segmentation_param->dnnOutputTensorSize = p_segmentation_param->inputWidth * p_segmentation_param->inputHeight;


    return E_PPL_OK;
}

/**
 * @brief createSegmentationData
 */
static void createSegmentationData(float *data_body, uint32_t data_size, SegmentationOutputTensor *segmentaion_output) {
    uint16_t y_old = UINT16_MAX;
    uint16_t y = 0;
    for (uint32_t i = 0; i < data_size; i++){
        y = i / PPL_SS_INPUT_TENSOR_WIDTH;
        if (y != y_old){
            y_old = y;
            segmentaion_output->classes.resize(y + 1);
        }
        segmentaion_output->classes[y].push_back(data_body[i]);
    }
    return;
}

/**
 * @brief analyzeSegmentationOutput
 */

static void analyzeSegmentationOutput(SegmentationOutputTensor out_tensor, SegmentationData *output_segmentation_data) {
    uint16_t height = out_tensor.classes.size();
    uint16_t width = out_tensor.classes.at(0).size();
    PPL_DBG_PRINTF("Height: %d", height);
    PPL_DBG_PRINTF("Width: %d", width);

    /* Extract scores */
    for (uint16_t y = 0; y < height; y++){
        output_segmentation_data->class_id_map.resize(y + 1);
        for (uint16_t x = 0; x < width; x++){
            output_segmentation_data->class_id_map[y].push_back(static_cast<uint16_t>(out_tensor.classes[x][y]));
        }
    }
    output_segmentation_data->height = height;
    output_segmentation_data->width = width;

    PPL_DBG_PRINTF("ClassIdMapLength: %lu",  output_segmentation_data->class_id_map.size() * output_segmentation_data->class_id_map.at(0).size());

    /**
     * @note The following settings are required when using score map.
    */
    output_segmentation_data->num_class_id = 0;
    PPL_DBG_PRINTF("NumClassId: %d",  output_segmentation_data->num_class_id);
    output_segmentation_data->score_map.clear();

    return;
}


/* Function to serialize Segmentation output tensor data into Flatbuffers.
*/
static void createSegmentationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const SegmentationData *segmentationData) {
    std::vector<uint16_t> w_class_id_map;
    for (uint16_t y = 0; y < segmentationData->class_id_map.size(); y++){
        for (uint16_t x = 0; x < segmentationData->class_id_map.at(0).size(); x++){
            w_class_id_map.push_back(segmentationData->class_id_map[y][x]);
        }
    }
    flatbuffers::Offset<flatbuffers::Vector<uint16_t>> class_id_map;
    if (!w_class_id_map.empty()){
        class_id_map = builder->CreateVector(w_class_id_map.data(), w_class_id_map.size());
    }

    std::vector<float> w_score_map;
    for (uint16_t z = 0; z < segmentationData->score_map.size(); z++){
        for (uint16_t y = 0; y < segmentationData->score_map.at(0).size(); y++){
            for (uint16_t x = 0; x < segmentationData->score_map.at(0).at(0).size(); x++){
                w_score_map.push_back(segmentationData->score_map[z][y][x]);
            }
        }
    }
    flatbuffers::Offset<flatbuffers::Vector<float>> score_map;
    if (!w_score_map.empty()){
        score_map = builder->CreateVector(w_score_map.data(), w_score_map.size());
    }

    auto out_data = SmartCamera::CreateSemanticSegmentationData(*builder,
                        segmentationData->height,
                        segmentationData->width,
                        class_id_map, 
                        segmentationData->num_class_id,
                        score_map);

    auto out_top = SmartCamera::CreateSemanticSegmentationTop(*builder, out_data);

    builder->Finish(out_top);

    return;
}
