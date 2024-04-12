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
#include <pthread.h>

#include "vision_app_public.h"
#include "analyzer_classification.h"
#include "classification_generated.h"

extern pthread_mutex_t g_libc_mutex;

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define PPL_ERR_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "E [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define PPL_WARN_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "W [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define PPL_INFO_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stdout, "I [VisionAPP] ");fprintf(stdout, fmt, ##__VA_ARGS__);fprintf(stdout, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define PPL_DBG_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "D [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n");pthread_mutex_unlock(&g_libc_mutex);
#define PPL_VER_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "V [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n");pthread_mutex_unlock(&g_libc_mutex);

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
    pthread_mutex_lock(&g_libc_mutex);
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    pthread_mutex_unlock(&g_libc_mutex);

    createClassificationFlatbuffer(builder, &output_classification_data);

    pthread_mutex_lock(&g_libc_mutex);
    uint8_t* buf_ptr = builder->GetBufferPointer();
    pthread_mutex_unlock(&g_libc_mutex);

    if (buf_ptr == NULL) {
        PPL_ERR_PRINTF("Error could not create Flatbuffer");

        pthread_mutex_lock(&g_libc_mutex);
        builder->Clear();
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        delete(builder);
        pthread_mutex_unlock(&g_libc_mutex);

        return E_PPL_OTHER;
    }

    pthread_mutex_lock(&g_libc_mutex);
    uint32_t buf_size = builder->GetSize();
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    p_out_param = (uint8_t *)SessMalloc(buf_size);
    pthread_mutex_unlock(&g_libc_mutex);

    if (p_out_param == NULL) {
        PPL_ERR_PRINTF("malloc failed for creating flatbuffer, malloc size=%d", buf_size);

        pthread_mutex_lock(&g_libc_mutex);
        builder->Clear();
        pthread_mutex_unlock(&g_libc_mutex);
    
        pthread_mutex_lock(&g_libc_mutex);
        delete(builder);
        pthread_mutex_unlock(&g_libc_mutex);

        return E_PPL_E_MEMORY_ERROR;
    }
    PPL_DBG_PRINTF("p_out_param = %p", p_out_param);

    /* Copy Data */
    pthread_mutex_lock(&g_libc_mutex);
    memcpy(p_out_param, buf_ptr,buf_size);
    pthread_mutex_unlock(&g_libc_mutex);

    *pp_out_buf = p_out_param;
    *p_out_size = buf_size;

    //Clean up
    pthread_mutex_lock(&g_libc_mutex);
    builder->Clear();
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    delete(builder);
    pthread_mutex_unlock(&g_libc_mutex);

    return E_PPL_OK;
}

/**
 * json_parse: get json data from json_object and update in local struct
 * 
 * @param param Pointer to json string containing PPL specific parameters
 * @return Success or failure EPPL_RESULT_CODE
 */
EPPL_RESULT_CODE json_parse(JSON_Value *root_value, PPL_ClsParam *p_cls_param) {
    PPL_DBG_PRINTF("PPL_Initialize: <json_parse>");
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
    int ret;

    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"dnn_output_classes");
    pthread_mutex_unlock(&g_libc_mutex);

     if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t dnn_output_classes = json_object_get_number(json_object(root_value), "dnn_output_classes");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ClassificationParamInit dnn_output_classes: %d", dnn_output_classes);
        p_cls_param->dnnOutputClasses = dnn_output_classes;
    } else {
        PPL_ERR_PRINTF("PPL_ClassificationParamInit: json file does not have \"dnn_output_classes\"");
        return E_PPL_INVALID_PARAM;
    }

    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"max_predictions");
    pthread_mutex_unlock(&g_libc_mutex);
    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t maxPredictions = (int)json_object_get_number(json_object(root_value), "max_predictions");
        pthread_mutex_unlock(&g_libc_mutex);

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

        pthread_mutex_lock(&g_libc_mutex);
        classification_output->scores.push_back(score);
        pthread_mutex_unlock(&g_libc_mutex);
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
    pthread_mutex_lock(&g_libc_mutex);
    num_of_predictions = out_tensor.scores.size();
    pthread_mutex_unlock(&g_libc_mutex);

    /* Extract scores */
    for (uint16_t i = 0; i < num_of_predictions; i++) {
        ClassificationItem item;
        item.index = i;

        pthread_mutex_lock(&g_libc_mutex);
        item.score = out_tensor.scores[i];
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        class_data.push_back(item);
        pthread_mutex_unlock(&g_libc_mutex);
    }

    pthread_mutex_lock(&g_libc_mutex);
    size_t size = class_data.size();
    pthread_mutex_unlock(&g_libc_mutex);

    if (size > 0) {
        pthread_mutex_lock(&g_libc_mutex);
        std::stable_sort(class_data.begin(), class_data.end(), [](const ClassificationItem &left, const ClassificationItem &right) {
            return left.score > right.score;
        });
        pthread_mutex_unlock(&g_libc_mutex);
    }

    for (uint16_t i = 0; i < cls_param.maxPredictions; i++) {
        pthread_mutex_lock(&g_libc_mutex);
        output_classification_data->v_classItem.push_back(class_data[i]);
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("Top[%d] = id: %d  score: %f", i, class_data[i].index, class_data[i].score);
    }

    return;
}

/* Function to serialize Classification output tensor data into Flatbuffers.
*/
static void createClassificationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const ClassificationData *classificationData) {

    std::vector<flatbuffers::Offset<SmartCamera::GeneralClassification>> gdata_vector;
    pthread_mutex_lock(&g_libc_mutex);
    unsigned int len = classificationData->v_classItem.size();
    pthread_mutex_unlock(&g_libc_mutex);

    for (unsigned int i = 0; i < len; i++) {
        PPL_DBG_PRINTF("class = %d, score = %f", classificationData->v_classItem[i].index,  classificationData->v_classItem[i].score);

        pthread_mutex_lock(&g_libc_mutex);
        auto general_data = SmartCamera::CreateGeneralClassification(*builder, classificationData->v_classItem[i].index, classificationData->v_classItem[i].score);
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        gdata_vector.push_back(general_data);
        pthread_mutex_unlock(&g_libc_mutex);
    }

    pthread_mutex_lock(&g_libc_mutex);
    auto v_bbox = builder->CreateVector(gdata_vector);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    auto class_data = SmartCamera::CreateClassificationData(*builder, v_bbox);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    auto out_data = SmartCamera::CreateClassificationTop(*builder, class_data);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    builder->Finish(out_data);
    pthread_mutex_unlock(&g_libc_mutex);

    return;
}
