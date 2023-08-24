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
#include <vector>

#include "vision_app_public.h"
#include "analyzer_classification.h"
#include "classification_generated.h"

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
typedef struct tagClassificationOutputTensor {
    std::vector<float> scores;
} ClassificationOutputTensor;

typedef struct tagClassificationItem {
    int index = 0;
    float score = 0;
} ClassificationItem;

typedef struct tagClassificationData {
    std::vector<ClassificationItem> v_classItem;
} ClassificationData;

/* -------------------------------------------------------- */
/* static                                                   */
/* -------------------------------------------------------- */
static EPPL_RESULT_CODE PPL_ClassificationParamInit(JSON_Value *root_value, PPL_ClsParam *p_cls_param);
static void createClassificationData(float* data_body, uint32_t data_size, ClassificationOutputTensor *classification_output);
static void analyzeClassificationOutput(ClassificationOutputTensor out_tensor, ClassificationData *output_classification_data, PPL_ClsParam cls_param);
static void createClassificationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const ClassificationData *classificationData);

/* -------------------------------------------------------- */
/* public function                                          */
/* -------------------------------------------------------- */
/**
 * @brief PPL_ClassificationAnalyze
 */
EPPL_RESULT_CODE PPL_ClassificationAnalyze(float *p_data, uint32_t in_size, void **pp_out_buf, uint32_t *p_out_size, PPL_ClsParam cls_param) {
    uint8_t *p_out_param = NULL;
    ClassificationOutputTensor classification_output;
    ClassificationData output_classification_data;

    PPL_DBG_PRINTF("PPL_ClassificationAnalyze");

    if (p_data == NULL) {
        PPL_ERR_PRINTF("PPL:Invalid param : pdata=NULL");
        return E_PPL_INVALID_PARAM;
    }

    if (in_size != cls_param.dnnOutputClasses) {
        PPL_ERR_PRINTF("PPL:Unexpected value for : in_size %d",in_size);
        PPL_ERR_PRINTF("PPL:Expected value for : in_size %d",cls_param.dnnOutputClasses);
        return E_PPL_INVALID_PARAM;
    }

    /* call interface process */
    createClassificationData(p_data, cls_param.dnnOutputClasses, &classification_output);

    /* call analyze process */
    analyzeClassificationOutput(classification_output, &output_classification_data, cls_param);

    /* serialize output data to flatbuffer */
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    createClassificationFlatbuffer(builder, &output_classification_data);

    uint8_t* buf_ptr = builder->GetBufferPointer();
    if (buf_ptr == NULL) {
        PPL_ERR_PRINTF("Error could not create Flatbuffer");
        builder->Clear();
        delete(builder);
        return E_PPL_OTHER;
    }

    uint32_t buf_size = builder->GetSize();
    p_out_param = (uint8_t *)SessMalloc(buf_size);
    if (p_out_param == NULL) {
        PPL_ERR_PRINTF("malloc failed for creating flatbuffer, malloc size=%d", buf_size);
        builder->Clear();
        delete(builder);
        return E_PPL_E_MEMORY_ERROR;
    }
    PPL_DBG_PRINTF("p_out_param = %p", p_out_param);

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
EPPL_RESULT_CODE json_parse(JSON_Value *root_value, PPL_ClsParam *p_cls_param) {
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
                    PPL_DBG_PRINTF("[PPL_PUBLIC] header_id is NULL");
                    return E_PPL_INVALID_PARAM;
                }
            } else {
                PPL_DBG_PRINTF("[PPL_PUBLIC] json file does not have header:id");
                return E_PPL_INVALID_PARAM;
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
                    PPL_DBG_PRINTF("[PPL_PUBLIC] header_version is NULL");
                    return E_PPL_INVALID_PARAM;
                }
            } else {
                PPL_DBG_PRINTF("[PPL_PUBLIC] json file does not have header:version");
                return E_PPL_INVALID_PARAM;
            }
        }
    }

    return PPL_ClassificationParamInit(root_value, p_cls_param);
}

/* -------------------------------------------------------- */
/* private function                                         */
/* -------------------------------------------------------- */
/**
 * @brief PPL_ClassificationParamInit
 */
static EPPL_RESULT_CODE PPL_ClassificationParamInit(JSON_Value *root_value, PPL_ClsParam *p_cls_param) {
    PPL_DBG_PRINTF("PPL_ClassificationParamInit");

     if (json_object_has_value(json_object(root_value),"dnn_output_classes")){
        uint16_t dnn_output_classes = json_object_get_number(json_object(root_value), "dnn_output_classes");
        PPL_DBG_PRINTF("PPL_ClassificationParamInit dnn_output_classes: %d", dnn_output_classes);
        p_cls_param->dnnOutputClasses = dnn_output_classes;
    } else {
        PPL_ERR_PRINTF("PPL_ClassificationParamInit: json file does not have \"dnn_output_classes\"");
        return E_PPL_INVALID_PARAM;
    }

    if (json_object_has_value(json_object(root_value),"max_predictions")) {
        uint16_t maxPredictions = (int)json_object_get_number(json_object(root_value), "max_predictions");
        PPL_DBG_PRINTF("PPL_ClassificationParamInit: max_predictions: %d", maxPredictions);

        /* check param */
        if (maxPredictions > p_cls_param->dnnOutputClasses) {
            PPL_WARN_PRINTF("max_predictions > max number of classes, set to max number of classes");
            p_cls_param->maxPredictions = p_cls_param->dnnOutputClasses;
        }
        else {
            p_cls_param->maxPredictions = maxPredictions;
        }
    } else {
        PPL_ERR_PRINTF("PPL_ClassificationParamInit: json file does not have max_predictions");
        return E_PPL_INVALID_PARAM;
    }

    return E_PPL_OK;
}

/**
 * @brief createClassificationData
 */
static void createClassificationData(float* data_body, uint32_t data_size, ClassificationOutputTensor *classification_output) {

    uint16_t size = data_size;
    for (uint16_t i = 0; i < size; i++) {
        float score = data_body[i];
        classification_output->scores.push_back(score);
    }

    return;
}

/**
 * @brief analyzeClassificationOutput
 */
static void analyzeClassificationOutput(ClassificationOutputTensor out_tensor, ClassificationData *output_classification_data, PPL_ClsParam cls_param) {

    uint16_t num_of_predictions;
    std::vector<ClassificationItem> class_data;

    /* Extract number of predictions */
    num_of_predictions = out_tensor.scores.size();

    /* Extract scores */
    for (uint16_t i = 0; i < num_of_predictions; i++) {
        ClassificationItem item;
        item.index = i;
        item.score = out_tensor.scores[i];
        class_data.push_back(item);
    }

    if (class_data.size() > 0) {
        std::stable_sort(class_data.begin(), class_data.end(), [](const ClassificationItem &left, const ClassificationItem &right) {
            return left.score > right.score;
        });
    }

    for (uint16_t i = 0; i < cls_param.maxPredictions; i++) {
        output_classification_data->v_classItem.push_back(class_data[i]);
        PPL_DBG_PRINTF("Top[%d] = id: %d  score: %f", i, class_data[i].index, class_data[i].score);
    }

    return;
}

/* Function to serialize Classification output tensor data into Flatbuffers.
*/
static void createClassificationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const ClassificationData *classificationData) {

    std::vector<flatbuffers::Offset<SmartCamera::GeneralClassification>> gdata_vector;
    for (unsigned int i = 0; i < classificationData->v_classItem.size(); i++) {
        PPL_DBG_PRINTF("class = %d, score = %f", classificationData->v_classItem[i].index,  classificationData->v_classItem[i].score);
        auto general_data = SmartCamera::CreateGeneralClassification(*builder, classificationData->v_classItem[i].index, classificationData->v_classItem[i].score);
        gdata_vector.push_back(general_data);
    }

    auto v_bbox = builder->CreateVector(gdata_vector);
    auto class_data = SmartCamera::CreateClassificationData(*builder, v_bbox);
    auto out_data = SmartCamera::CreateClassificationTop(*builder, class_data);

    builder->Finish(out_data);

    return;
}
