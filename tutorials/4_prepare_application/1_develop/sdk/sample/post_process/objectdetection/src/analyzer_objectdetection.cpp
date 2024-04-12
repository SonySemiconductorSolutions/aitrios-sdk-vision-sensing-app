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
#include <pthread.h>
#include <vector>

#include "vision_app_public.h"
#include "analyzer_objectdetection.h"
#include "objectdetection_generated.h"

extern pthread_mutex_t g_libc_mutex;

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define PPL_DNN_OUTPUT_TENSOR_SIZE(dnnOutputDetections)  ((dnnOutputDetections * 4) + dnnOutputDetections + dnnOutputDetections + 1)    // bbox(dnnOutputDetections*4)+class(dnnOutputDetections)+scores(dnnOutputDetections)+numOfDetections(1) 

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
typedef struct tagBbox {
    float x_min;
    float y_min;
    float x_max;
    float y_max;
} Bbox;

typedef struct tagObjectDetectionSsdOutputTensor {
    float numOfDetections;
    std::vector<Bbox> bboxes;
    std::vector<float> scores;
    std::vector<float> classes;
} ObjectDetectionSsdOutputTensor;

typedef struct tagPPL_Bbox {
    uint16_t    m_xmin;
    uint16_t    m_ymin;
    uint16_t    m_xmax;
    uint16_t    m_ymax;
} PPL_Bbox;

typedef struct tagObjectDetectionSsdData {
    uint8_t numOfDetections = 0;
    std::vector<PPL_Bbox> v_bbox;
    std::vector<float> v_scores;
    std::vector<uint8_t> v_classes;
} ObjectDetectionSsdData;

/* -------------------------------------------------------- */
/* static                                                   */
/* -------------------------------------------------------- */
static int createObjectDetectionSsdData(float *data_body, uint32_t detect_num, ObjectDetectionSsdOutputTensor *objectdetection_output);
static void analyzeObjectDetectionSsdOutput(ObjectDetectionSsdOutputTensor out_tensor, ObjectDetectionSsdData *output_objectdetection_data, PPL_SsdParam ssd_param);
static void createSSDOutputFlatbuffer(flatbuffers::FlatBufferBuilder *builder, const ObjectDetectionSsdData *ssdData);
static EPPL_RESULT_CODE PPL_ObjectDetectionSsdParamInit(JSON_Value *root_value, PPL_SsdParam *p_ssd_param);

/* -------------------------------------------------------- */
/* public function                                          */
/* -------------------------------------------------------- */
/**
 * @brief PPL_ObjectDetectionSsdAnalyze
 */
EPPL_RESULT_CODE PPL_ObjectDetectionSsdAnalyze(float *p_data, uint32_t in_size, void **pp_out_buf,  uint32_t *p_out_size, PPL_SsdParam ssd_param) {

    uint8_t *p_out_param = NULL;
    ObjectDetectionSsdOutputTensor objectdetection_output;
    ObjectDetectionSsdData output_objectdetection_data;

    PPL_DBG_PRINTF("PPL_ObjectDetectionSsdAnalyze");

    if (p_data == NULL) {
        PPL_ERR_PRINTF("PPL:Invalid param : pdata=NULL");
        return E_PPL_INVALID_PARAM;
    }

    if (in_size != (uint32_t)(PPL_DNN_OUTPUT_TENSOR_SIZE(ssd_param.dnnOutputDetections))) {
        PPL_ERR_PRINTF("PPL:Unexpected value for : in_size %d",in_size);
        PPL_ERR_PRINTF("PPL:Expected value for : in_size %d",(uint32_t)(PPL_DNN_OUTPUT_TENSOR_SIZE(ssd_param.dnnOutputDetections)));
        return E_PPL_INVALID_PARAM;
    }

    /* call interface process */
    int ret = createObjectDetectionSsdData(p_data, ssd_param.dnnOutputDetections, &objectdetection_output);
    if (ret != 0) {
        PPL_ERR_PRINTF("PPL: Error in createObjectDetectionData");
        return E_PPL_OTHER;
    }

    /* call analyze process */
    analyzeObjectDetectionSsdOutput(objectdetection_output, &output_objectdetection_data, ssd_param);

    /* Serialize Data to FLatbuffers */ 
    pthread_mutex_lock(&g_libc_mutex);
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    pthread_mutex_unlock(&g_libc_mutex);

    createSSDOutputFlatbuffer(builder,&output_objectdetection_data);

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
    memcpy(p_out_param, buf_ptr, buf_size);
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
EPPL_RESULT_CODE json_parse(JSON_Value *root_value, PPL_SsdParam *p_ssd_param) {
    PPL_DBG_PRINTF("PPL_Initialize: <json_parse>");
    return PPL_ObjectDetectionSsdParamInit(root_value, p_ssd_param);
}

/* -------------------------------------------------------- */
/* private function                                         */
/* -------------------------------------------------------- */
static EPPL_RESULT_CODE PPL_ObjectDetectionSsdParamInit(JSON_Value *root_value, PPL_SsdParam *p_ssd_param) {
    int ret;

    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"dnn_output_detections");
    pthread_mutex_unlock(&g_libc_mutex);

    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t dnn_output_detections = json_object_get_number(json_object(root_value), "dnn_output_detections");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ObjectDetectionParamInit dnn_output_detections: %d", dnn_output_detections);
        p_ssd_param->dnnOutputDetections = dnn_output_detections;
    } else {
        PPL_ERR_PRINTF("PPL_ObjectDetectionParamInit: json file does not have parameter \"dnn_output_detections\"");
        return E_PPL_INVALID_PARAM;
    }

    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"max_detections");
    pthread_mutex_unlock(&g_libc_mutex);

    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t maxDetections = (int)json_object_get_number(json_object(root_value), "max_detections");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ObjectDetectionParamInit max_detections: %d", maxDetections);
        if (maxDetections > p_ssd_param->dnnOutputDetections) {
            PPL_WARN_PRINTF("max_detections > max number of classes, set to max number of classes");
            p_ssd_param->maxDetections = p_ssd_param->dnnOutputDetections;
        } else {
            p_ssd_param->maxDetections = maxDetections;
        }
    } else {
        PPL_ERR_PRINTF("PPL_ObjectDetectionParamInit json file does not have max_detections");
        return E_PPL_INVALID_PARAM;
    }

    pthread_mutex_lock(&g_libc_mutex);
    json_object_has_value(json_object(root_value),"threshold");
    pthread_mutex_unlock(&g_libc_mutex);

    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        float threshold = json_object_get_number(json_object(root_value), "threshold");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ObjectDetectionParamInit threshold: %lf", threshold);
        if (threshold < 0.0 || threshold > 1.0) {
            PPL_WARN_PRINTF("threshold value out of range, set to default threshold");
            p_ssd_param->threshold = PPL_DEFAULT_THRESHOLD;
        } else {
            p_ssd_param->threshold = threshold;
        }
    } else {
        PPL_ERR_PRINTF("PPL_ObjectDetectionParamInit: json file does not have threshold");
        return E_PPL_INVALID_PARAM;
    }
    
    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"input_width");
    pthread_mutex_unlock(&g_libc_mutex);

    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t input_width = json_object_get_number(json_object(root_value), "input_width");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ObjectDetectionParamInit input_width: %d", input_width);
        p_ssd_param->inputWidth = input_width;
    } else {
        PPL_ERR_PRINTF("PPL_ObjectDetectionParamInit: json file does not have input_width");
        return E_PPL_INVALID_PARAM;
    }

    pthread_mutex_lock(&g_libc_mutex);
    ret = json_object_has_value(json_object(root_value),"input_height");
    pthread_mutex_unlock(&g_libc_mutex);

    if (ret) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t input_height = json_object_get_number(json_object(root_value), "input_height");
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("PPL_ObjectDetectionParamInit input_height: %d", input_height);
        p_ssd_param->inputHeight = input_height;
    } else {
        PPL_ERR_PRINTF("PPL_ObjectDetectionParamInit: json file does not have \"input_height\"");
        return E_PPL_INVALID_PARAM;
    }

    return E_PPL_OK;
}

/**
 * @brief createObjectDetectionSsdData
 */
static int createObjectDetectionSsdData(float *data_body, uint32_t detect_num, ObjectDetectionSsdOutputTensor *objectdetection_output) {

    float* out_data = data_body;
    uint32_t count = 0;
    std::vector<Bbox> v_bbox;
    std::vector<float> v_scores;
    std::vector<float> v_classes;

    /* Extract number of Detections */
    uint8_t totalDetections = (uint8_t)detect_num;

    if ((count + (totalDetections * 4))> PPL_DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
        PPL_ERR_PRINTF("Invalid count index %d",count);
        return -1;
    }

    //Extract bounding box co-ordinates
    for (uint8_t i = 0; i < totalDetections; i++) {
        Bbox bbox;
        bbox.y_min = out_data[count + i];
        bbox.x_min = out_data[count + i + (1 * totalDetections)];
        bbox.y_max = out_data[count + i + (2 * totalDetections)];
        bbox.x_max = out_data[count + i + (3 * totalDetections)];
        pthread_mutex_lock(&g_libc_mutex);
        v_bbox.push_back(bbox);
        pthread_mutex_unlock(&g_libc_mutex);
    }
    count += (totalDetections * 4);

    //Extract class indices
    for (uint8_t i = 0; i < totalDetections; i++) {
        if (count > PPL_DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
            PPL_ERR_PRINTF("Invalid count index %d",count);
            return -1;
        }
        float class_index;
        class_index = out_data[count];

        pthread_mutex_lock(&g_libc_mutex);
        v_classes.push_back(class_index);
        pthread_mutex_unlock(&g_libc_mutex);

        count++;
    }

    //Extract scores
    for (uint8_t i = 0; i < totalDetections; i++) {
        if (count > PPL_DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
            PPL_ERR_PRINTF("Invalid count index %d",count);
            return -1;
        }
        float score;
        score = out_data[count];

        pthread_mutex_lock(&g_libc_mutex);
        v_scores.push_back(score);
        pthread_mutex_unlock(&g_libc_mutex);

        count++;
    }

    if (count > PPL_DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
        PPL_ERR_PRINTF("Invalid count index %d",count);
        return -1;
    }

    //Extract number of Detections
    uint8_t numOfDetections = (uint8_t) out_data[count];

    if (numOfDetections > totalDetections) {
         PPL_WARN_PRINTF("Unexpected value for numOfDetections: %d, setting it to %d",numOfDetections,totalDetections);
         numOfDetections = totalDetections;
    }

    objectdetection_output->numOfDetections = numOfDetections;

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_output->bboxes = v_bbox;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_output->scores = v_scores;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_output->classes = v_classes;
    pthread_mutex_unlock(&g_libc_mutex);

    return 0;
}

/**
 * @brief analyzeObjectDetectionSsdOutput
 */
static void analyzeObjectDetectionSsdOutput(ObjectDetectionSsdOutputTensor out_tensor, ObjectDetectionSsdData *output_objectdetection_data, PPL_SsdParam ssd_param) {

    uint8_t num_of_detections;
    uint8_t detections_above_threshold = 0;
    std::vector<PPL_Bbox> v_bbox;
    std::vector<float> v_scores;
    std::vector<uint8_t> v_classes;
    ObjectDetectionSsdData objectdetection_data;

    /* Extract number of detections */
    num_of_detections = (uint8_t)out_tensor.numOfDetections;

    for (uint8_t i = 0; i < num_of_detections; i++) {

        /* Extract scores */
        float score;

        pthread_mutex_lock(&g_libc_mutex);
        score = out_tensor.scores[i];
        pthread_mutex_unlock(&g_libc_mutex);

        /* Filter Detections */
        if (score < ssd_param.threshold) {
            continue;
        } else {
            pthread_mutex_lock(&g_libc_mutex);
            v_scores.push_back(score);
            pthread_mutex_unlock(&g_libc_mutex);

            /* Extract bounding box co-ordinates */
            PPL_Bbox bbox;
            pthread_mutex_lock(&g_libc_mutex);
            bbox.m_xmin = (uint16_t)(round((out_tensor.bboxes[i].x_min) * (ssd_param.inputWidth - 1)));
            pthread_mutex_unlock(&g_libc_mutex);

            pthread_mutex_lock(&g_libc_mutex);
            bbox.m_ymin = (uint16_t)(round((out_tensor.bboxes[i].y_min) * (ssd_param.inputHeight  - 1)));
            pthread_mutex_unlock(&g_libc_mutex);

            pthread_mutex_lock(&g_libc_mutex);
            bbox.m_xmax = (uint16_t)(round((out_tensor.bboxes[i].x_max) * (ssd_param.inputWidth  - 1)));
            pthread_mutex_unlock(&g_libc_mutex);

            pthread_mutex_lock(&g_libc_mutex);
            bbox.m_ymax = (uint16_t)(round((out_tensor.bboxes[i].y_max) * (ssd_param.inputHeight - 1)));
            pthread_mutex_unlock(&g_libc_mutex);

            pthread_mutex_lock(&g_libc_mutex);
            v_bbox.push_back(bbox);
            pthread_mutex_unlock(&g_libc_mutex);

           /* Extract classes */
            uint8_t class_index;
            pthread_mutex_lock(&g_libc_mutex);
            class_index = (uint8_t)out_tensor.classes[i];
            pthread_mutex_unlock(&g_libc_mutex);

            pthread_mutex_lock(&g_libc_mutex);
            v_classes.push_back(class_index);
            pthread_mutex_unlock(&g_libc_mutex);

            detections_above_threshold++;
        }
    }

    objectdetection_data.numOfDetections = detections_above_threshold;

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_data.v_bbox = v_bbox;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_data.v_scores = v_scores;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    objectdetection_data.v_classes = v_classes;
    pthread_mutex_unlock(&g_libc_mutex);

    //objectdetection_data = getActualDetections(objectdetection_data);

    if (objectdetection_data.numOfDetections > ssd_param.maxDetections) {
        objectdetection_data.numOfDetections = ssd_param.maxDetections;

        pthread_mutex_lock(&g_libc_mutex);
        objectdetection_data.v_bbox.resize(ssd_param.maxDetections);
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        objectdetection_data.v_classes.resize(ssd_param.maxDetections);
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        objectdetection_data.v_scores.resize(ssd_param.maxDetections);
        pthread_mutex_unlock(&g_libc_mutex);
    }

    output_objectdetection_data->numOfDetections = objectdetection_data.numOfDetections;

    pthread_mutex_lock(&g_libc_mutex);
    output_objectdetection_data->v_bbox = objectdetection_data.v_bbox;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    output_objectdetection_data->v_scores = objectdetection_data.v_scores;
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    output_objectdetection_data->v_classes = objectdetection_data.v_classes;
    pthread_mutex_unlock(&g_libc_mutex);

    PPL_DBG_PRINTF("number of detections = %d", objectdetection_data.numOfDetections);
    num_of_detections = objectdetection_data.numOfDetections;
    for (int i = 0; i < num_of_detections; i++) {
        pthread_mutex_lock(&g_libc_mutex);
        uint16_t xmin = objectdetection_data.v_bbox[i].m_xmin;
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        uint16_t ymin = objectdetection_data.v_bbox[i].m_ymin;
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        uint16_t xmax = objectdetection_data.v_bbox[i].m_xmax;
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        uint16_t ymax = objectdetection_data.v_bbox[i].m_ymax;
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("v_bbox[%d] :[x_min,y_min,x_max,y_max] = [%d,%d,%d,%d]", i, xmin, ymin, xmax, ymax);
    }
    for (int i = 0; i < num_of_detections; i++) {
        pthread_mutex_lock(&g_libc_mutex);
        float score = objectdetection_data.v_scores[i];
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("scores[%d] = %f", i, score);
    }
    for (int i = 0; i < num_of_detections; i++) {
        pthread_mutex_lock(&g_libc_mutex);
        uint8_t class_indice = objectdetection_data.v_classes[i];
        pthread_mutex_unlock(&g_libc_mutex);

        PPL_DBG_PRINTF("class_indices[%d/%d] = %d", i, num_of_detections, class_indice);
    }

    return;
}

/* Function to serialize SSD MobilenetV1 output tensor data into Flatbuffers.
*/
static void createSSDOutputFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const ObjectDetectionSsdData* ssdData) {
    std::vector<flatbuffers::Offset<SmartCamera::GeneralObject>> gdata_vector;

    PPL_DBG_PRINTF("createFlatbuffer");
    uint8_t numOfDetections = ssdData->numOfDetections;
    for (uint8_t i = 0; i < numOfDetections; i++) {
        PPL_DBG_PRINTF("left = %d, top = %d, right = %d, bottom = %d, class = %d, score = %f", ssdData->v_bbox[i].m_xmin, ssdData->v_bbox[i].m_ymin, ssdData->v_bbox[i].m_xmax, ssdData->v_bbox[i].m_ymax, ssdData->v_classes[i], ssdData->v_scores[i]);
        pthread_mutex_lock(&g_libc_mutex);
        auto bbox_data = SmartCamera::CreateBoundingBox2d(*builder, ssdData->v_bbox[i].m_xmin, \
            ssdData->v_bbox[i].m_ymin, \
            ssdData->v_bbox[i].m_xmax, \
            ssdData->v_bbox[i].m_ymax);
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        auto general_data = SmartCamera::CreateGeneralObject(*builder, ssdData->v_classes[i], SmartCamera::BoundingBox_BoundingBox2d, bbox_data.Union(), ssdData->v_scores[i]);
        pthread_mutex_unlock(&g_libc_mutex);

        pthread_mutex_lock(&g_libc_mutex);
        gdata_vector.push_back(general_data);
        pthread_mutex_unlock(&g_libc_mutex);
    }

    pthread_mutex_lock(&g_libc_mutex);
    auto v_bbox = builder->CreateVector(gdata_vector);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    auto od_data = SmartCamera::CreateObjectDetectionData(*builder, v_bbox);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    auto out_data = SmartCamera::CreateObjectDetectionTop(*builder, od_data);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    builder->Finish(out_data);
    pthread_mutex_unlock(&g_libc_mutex);

    return;
}
