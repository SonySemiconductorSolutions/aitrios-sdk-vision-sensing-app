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

#include <string.h>
#include <math.h>

#include "analyzer_switch_dnn.h"

extern pthread_mutex_t g_libc_mutex;

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define DNN_OUTPUT_TENSOR_SIZE(dnnOutputDetections)  ((dnnOutputDetections * 4) + dnnOutputDetections + dnnOutputDetections + 1)    // bbox(dnnOutputDetections*4)+class(dnnOutputDetections)+scores(dnnOutputDetections)+numOfDetections(1)
#define NETWORK_ID_LEN (6)
#define BIRD_CLASS (15)

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define ERR_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "E [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define WARN_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "W [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define INFO_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stdout, "I [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define DBG_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "D [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define VER_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "V [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define MUTEX_LOCK() pthread_mutex_lock(&g_libc_mutex);
#define MUTEX_UNLOCK() pthread_mutex_unlock(&g_libc_mutex);

/////////////////////////////////
// AnalyzerBase
/////////////////////////////////

AnalyzerBase::AnalyzerBase(){
    pthread_mutex_init(&mutex_, nullptr);
}
AnalyzerBase::~AnalyzerBase(){
    pthread_mutex_destroy(&mutex_);
}
int AnalyzerBase::Lock() {
    return pthread_mutex_lock(&mutex_);
}
int AnalyzerBase::Unlock() {
    return pthread_mutex_unlock(&mutex_); 
}

/////////////////////////////////
// AnalyzerCommon
/////////////////////////////////

AnalyzerBase::ResultCode AnalyzerCommon::ValidateParam(const void* param)
{
    DBG_PRINTF("AnalyzerCommon::ValidateParam");
    AnalyzerBase::ScopeLock lock(this);

    // parse the json parameter
    JSON_Value *root_value;
    MUTEX_LOCK();
    root_value = json_parse_string((char *)param);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    JSON_Value_Type type = json_value_get_type(root_value);
    MUTEX_UNLOCK();
    if (type != JSONObject) {
        MUTEX_LOCK();
        json_value_free(root_value);
        MUTEX_UNLOCK();
        ERR_PRINTF("AnalyzerCommon::ValidateParam: Invalid param");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    // check id and version
    const char *p              = nullptr;
    const char *ppl_id_version = ID_VERSION;
    int ret;
    MUTEX_LOCK();
    ret = json_object_has_value(json_object(root_value),"header");
    MUTEX_UNLOCK();
    if (ret) {
        MUTEX_LOCK();
        ret = json_object_has_value_of_type(json_object(root_value),"header",JSONObject);
        MUTEX_UNLOCK();
        if (ret) {
            MUTEX_LOCK();
            JSON_Object *header = json_object_get_object(json_object(root_value), "header");
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            ret = json_object_has_value(header,"id");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                p = json_object_get_string(header, "id");
                MUTEX_UNLOCK();
                if (p != nullptr) {
                    MUTEX_LOCK();
                    ret = strncmp(ppl_id_version, p, 2); 
                    MUTEX_UNLOCK();
                    if (ret == 0) {
                        DBG_PRINTF("ValidateParam: header_id = %s",p);
                    } else {
                        ERR_PRINTF("ValidateParam: Unexpected header_id = %s",p);
                        return AnalyzerBase::ResultCode::kInvalidParam;
                    }
                } else {
                    ERR_PRINTF("ValidateParam: header_id is NULL");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
            } else {
                ERR_PRINTF("ValidateParam: json file does not have header:id");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }

            MUTEX_LOCK();
            ret = json_object_has_value(header,"version");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                p = json_object_get_string(header, "version");
                MUTEX_UNLOCK();
                if (p != nullptr) {
                    MUTEX_LOCK();
                    ret = strncmp(&ppl_id_version[3], p, 8);
                    MUTEX_UNLOCK();
                    if (ret == 0) {
                        DBG_PRINTF("ValidateParam: header_version p = %s ppl_id_version=%s",p,&ppl_id_version[3]);
                    } else {
                        ERR_PRINTF("ValidateParam: Unexpected header_version = %s",p);
                        return AnalyzerBase::ResultCode::kInvalidParam;
                    }
                } else {
                    ERR_PRINTF("ValidateParam: header_version is NULL");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
            } else {
                ERR_PRINTF("ValidateParam: json file does not have header:version");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }
        }
    }

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerCommon::SetValidatedParam(const void* param)
{
    AnalyzerBase::ScopeLock lock(this);
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerCommon::ClearValidatingParam()
{
    AnalyzerBase::ScopeLock lock(this);
    DoClearValidatingParam();
    return AnalyzerBase::ResultCode::kOk;
}

void AnalyzerCommon::DoClearValidatingParam()
{
    // nothing to do.
}

AnalyzerBase::ResultCode AnalyzerCommon::Analyze(const float* p_data, uint32_t size, uint64_t trace_id)
{
    AnalyzerBase::ScopeLock lock(this);
    return AnalyzerBase::ResultCode::kInvalidParam;
}
AnalyzerBase::ResultCode AnalyzerCommon::Serialize(void** out_buf, uint32_t* size, const Allocator* allocator)
{
    AnalyzerBase::ScopeLock lock(this);
    return AnalyzerBase::ResultCode::kInvalidParam;
}

AnalyzerBase::ResultCode AnalyzerCommon::GetNetworkId(uint32_t& network_id) const
{
    return AnalyzerBase::ResultCode::kInvalidParam;
}

/////////////////////////////////
// AnalyzerOd
/////////////////////////////////

AnalyzerBase::ResultCode AnalyzerOd::ValidateParam(const void* param)
{
    DBG_PRINTF("AnalyzerOd::ValidateParam");
    AnalyzerBase::ScopeLock lock(this);

    // parse the json parameter
    JSON_Value *root_value;
    MUTEX_LOCK();
    root_value = json_parse_string((char *)param);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    JSON_Value_Type type = json_value_get_type(root_value);
    MUTEX_UNLOCK();
    if (type != JSONObject) {
        MUTEX_LOCK();
        json_value_free(root_value);
        MUTEX_UNLOCK();
        ERR_PRINTF("AnalyzerOd::ValidateParam: Invalid param");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    if (!validating_param_) {
        validating_param_ = new PPLParam;
    }
    AnalyzerBase::ResultCode ret = ObjectDetectionParamInit(root_value, validating_param_);
    if (ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("AnalyzerOd::ValidateParam: Get json_parse Fail Err[%d]", ret);
        MUTEX_LOCK();
        json_value_free(root_value);
        MUTEX_UNLOCK();
        DoClearValidatingParam();
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    MUTEX_LOCK();
    json_value_free(root_value);
    MUTEX_UNLOCK();

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::SetValidatedParam(const void* param)
{
    DBG_PRINTF("AnalyzerOd::SetValidatedParam");
    AnalyzerBase::ScopeLock lock(this);

    if (!validating_param_) {
        return AnalyzerBase::ResultCode::kOtherError;   
    }

    // set ppl parameter
    param_ = *validating_param_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::ClearValidatingParam()
{
    DBG_PRINTF("AnalyzerOd::ClearValidatingParam");
    AnalyzerBase::ScopeLock lock(this);
    DoClearValidatingParam();
    return AnalyzerBase::ResultCode::kOk;
}

void AnalyzerOd::DoClearValidatingParam()
{
    if (validating_param_) {
        delete validating_param_;
        validating_param_ = nullptr;
    }
}

AnalyzerBase::ResultCode AnalyzerOd::Analyze(const float* p_data, uint32_t size, uint64_t trace_id)
{
    DBG_PRINTF("AnalyzerOd::Analyze");
    AnalyzerBase::ScopeLock lock(this);

    PPLParam ppl_parameter;
    AnalyzerOd::GetParam(ppl_parameter);

    if (p_data == nullptr) {
        ERR_PRINTF("AnalyzerOd::Analyze: Invalid param pdata=nullptr");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    if (size != (uint32_t)(DNN_OUTPUT_TENSOR_SIZE(ppl_parameter.dnn_output_detections_))) {
        ERR_PRINTF("AnalyzerOd::Analyze: Unexpected value for output tensor size %u", size);
        ERR_PRINTF("AnalyzerOd::Analyze: Expected value for output tensor size %u", (uint32_t)(DNN_OUTPUT_TENSOR_SIZE(ppl_parameter.dnn_output_detections_)));
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    // call interface process
    DetectionOutputTensor objectdetection_output;
    int ret = createDetectionData(p_data, ppl_parameter.dnn_output_detections_, &objectdetection_output);
    if (ret != 0) {
        ERR_PRINTF("AnalyzerOd::Analyze: Error in createObjectDetectionData");
        return AnalyzerBase::ResultCode::kOtherError;
    }

    // call analyze process
    DetectionData output_objectdetection_data;
    analyzeDetectionOutput(objectdetection_output, &output_objectdetection_data, ppl_parameter, trace_id);

    data_ = output_objectdetection_data;
    
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::Serialize(void** out_buf, uint32_t* size, const Allocator* allocator)
{
    DBG_PRINTF("AnalyzerOd::Serialize");
    AnalyzerBase::ScopeLock lock(this);

    DetectionData detection_data;
    AnalyzerOd::GetAnalyzedData(detection_data);

    MUTEX_LOCK();
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    MUTEX_UNLOCK();
    createSSDOutputFlatbuffer(builder,&detection_data);

    MUTEX_LOCK();
    uint8_t* buf_ptr = builder->GetBufferPointer();
    MUTEX_UNLOCK();
    if (buf_ptr == nullptr) {
        ERR_PRINTF("Error could not create Flatbuffer");
        MUTEX_LOCK();
        builder->Clear();
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        delete(builder);
        MUTEX_UNLOCK();
        return AnalyzerBase::ResultCode::kOtherError;
    }

    MUTEX_LOCK();
    uint32_t buf_size = builder->GetSize();
    MUTEX_UNLOCK();
    uint8_t *p_out_param = nullptr;
    p_out_param = (uint8_t *)allocator->Malloc(buf_size);
    if (p_out_param == nullptr) {
        ERR_PRINTF("malloc failed for creating flatbuffer, malloc size=%u", buf_size);
        MUTEX_LOCK();
        builder->Clear();
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        delete(builder);
        MUTEX_UNLOCK();
        return AnalyzerBase::ResultCode::kMemoryError;
    }
    DBG_PRINTF("p_out_param = %p", p_out_param);

    // Copy Data
    MUTEX_LOCK();
    memcpy(p_out_param, buf_ptr, buf_size);
    MUTEX_UNLOCK();
    *out_buf = p_out_param;
    *size = buf_size;

    // Clean up
    MUTEX_LOCK();
    builder->Clear();
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    delete(builder);
    MUTEX_UNLOCK();

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::GetNetworkId(uint32_t& network_id) const
{
    network_id = network_id_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::GetAnalyzedData(DetectionData& data) const
{
    data = data_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::GetInputTensorSize(uint16_t& width, uint16_t& height) const
{
    width = param_.input_width_;
    height = param_.input_height_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::SetNetworkId(uint32_t network_id)
{
    network_id_ = network_id;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::ObjectDetectionParamInit(JSON_Value *root_value, AnalyzerOd::PPLParam *p_param)
{
    int ret;
    MUTEX_LOCK();
    ret = json_object_has_value(json_object(root_value),"models");
    MUTEX_UNLOCK();
    if (ret) {
        MUTEX_LOCK();
        JSON_Object* models = json_object_get_object(json_object(root_value), "models");
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        ret = json_object_has_value(models,"detection_bird");
        MUTEX_UNLOCK();
        if (ret) {
            MUTEX_LOCK();
            JSON_Object* detection_param = json_object_get_object(models, "detection_bird");
            MUTEX_UNLOCK();

            // Check network id
            MUTEX_LOCK();
            ret = json_object_has_value(detection_param,"network_id");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                const char* network_id_str = json_object_get_string(detection_param,"network_id");
                MUTEX_UNLOCK();
                if (network_id_str == nullptr) {
                    ERR_PRINTF("ObjectDetectionParamInit: network_id is invalid");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
                MUTEX_LOCK();
                ret = strnlen(network_id_str, 7);
                MUTEX_UNLOCK();
                if (ret != NETWORK_ID_LEN) {
                    ERR_PRINTF("ObjectDetectionParamInit: network_id must be six characters");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
                char* end;
                MUTEX_LOCK();
                long int network_id = strtol(network_id_str, &end, 16);
                MUTEX_UNLOCK();
                if (network_id == 0) {
                    ERR_PRINTF("ObjectDetectionParamInit: network_id is invalid");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                } else {
                    SetNetworkId(network_id);
                    DBG_PRINTF("ObjectDetectionParamInit network_id :%lx", network_id);
                }
            } else {
                ERR_PRINTF("ObjectDetectionParamInit: json file does not have network_id");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }

            // Check param
            MUTEX_LOCK();
            ret = json_object_has_value(detection_param,"param");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                JSON_Object* param = json_object_get_object(detection_param, "param");
                MUTEX_UNLOCK();

                MUTEX_LOCK();
                ret = json_object_has_value(param,"dnn_output_detections");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t dnn_output_detections = json_object_get_number(param, "dnn_output_detections");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ObjectDetectionParamInit dnn_output_detections: %d", dnn_output_detections);
                    p_param->dnn_output_detections_ = dnn_output_detections;
                } else {
                    ERR_PRINTF("ObjectDetectionParamInit: json file does not have parameter dnn_output_detections");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }

                MUTEX_LOCK();
                ret = json_object_has_value(param,"max_detections");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t maxDetections = (int)json_object_get_number(param, "max_detections");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ObjectDetectionParamInit max_detections: %d", maxDetections);
                    if (maxDetections > p_param->dnn_output_detections_) {
                        WARN_PRINTF("max_detections > max number of classes, set to max number of classes");
                        p_param->max_detections_ = p_param->dnn_output_detections_;
                    } else {
                        p_param->max_detections_ = maxDetections;
                    }
                } else {
                    ERR_PRINTF("ObjectDetectionParamInit: json file does not have max_detections");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }

                MUTEX_LOCK();
                ret = json_object_has_value(param,"threshold");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    float threshold = json_object_get_number(param, "threshold");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ObjectDetectionParamInit threshold: %lf", threshold);
                    if (threshold < 0.0 || threshold > 1.0) {
                        ERR_PRINTF("ObjectDetectionParamInit: threshold value out of range");
                        return AnalyzerBase::ResultCode::kInvalidParam;
                    } else {
                        p_param->threshold_ = threshold;
                    }
                } else {
                    ERR_PRINTF("ObjectDetectionParamInit: json file does not have threshold");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }

                MUTEX_LOCK();
                ret = json_object_has_value(param,"input_width");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t input_width = json_object_get_number(param, "input_width");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ObjectDetectionParamInit input_width: %d", input_width);
                    p_param->input_width_ = input_width;
                } else {
                    ERR_PRINTF("ObjectDetectionParamInit: json file does not have input_width");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }

                MUTEX_LOCK();
                ret = json_object_has_value(param,"input_height");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t input_height = json_object_get_number(param, "input_height");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ObjectDetectionParamInit input_height: %d", input_height);
                    p_param->input_height_ = input_height;
                } else {
                    ERR_PRINTF("ObjectDetectionParamInit: json file does not have input_height");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
            } else {
                ERR_PRINTF("ObjectDetectionParamInit: json file does not have param");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }
        } else {
            ERR_PRINTF("ObjectDetectionParamInit: json file does not have detection_bird");
            return AnalyzerBase::ResultCode::kInvalidParam;
        }
    } else {
        ERR_PRINTF("ObjectDetectionParamInit: json file does not have models");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    return  AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerOd::GetParam(AnalyzerOd::PPLParam& param) const
{
    param = param_;
    return ResultCode::kOk;
}

int AnalyzerOd::createDetectionData(const float *data_body, uint32_t detect_num, AnalyzerOd::DetectionOutputTensor *objectdetection_output)
{
    const float* out_data = data_body;
    uint32_t     count    = 0;
    std::vector<OutputTensorBbox> v_bbox;
    std::vector<float>            v_scores;
    std::vector<float>            v_classes;

    // Extract number of Detections
    uint8_t totalDetections = (uint8_t)detect_num;

    if ((count + (totalDetections * 4))> DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
        ERR_PRINTF("Invalid count index %u", count);
        return -1;
    }

    // Extract bounding box co-ordinates
    for (uint8_t i = 0; i < totalDetections; i++) {
        OutputTensorBbox bbox;
        bbox.y_min_ = out_data[count + i];
        bbox.x_min_ = out_data[count + i + (1 * totalDetections)];
        bbox.y_max_ = out_data[count + i + (2 * totalDetections)];
        bbox.x_max_ = out_data[count + i + (3 * totalDetections)];
        MUTEX_LOCK();
        v_bbox.push_back(bbox);
        MUTEX_UNLOCK();
    }
    count += (totalDetections * 4);

    // Extract class indices
    for (uint8_t i = 0; i < totalDetections; i++) {
        if (count > DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
            ERR_PRINTF("Invalid count index %u", count);
            return -1;
        }
        float class_index;
        class_index = out_data[count];
        MUTEX_LOCK();
        v_classes.push_back(class_index);
        MUTEX_UNLOCK();
        count++;
    }

    // Extract scores
    for (uint8_t i = 0; i < totalDetections; i++) {
        if (count > DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
            ERR_PRINTF("Invalid count index %u", count);
            return -1;
        }
        float score;
        score = out_data[count];
        MUTEX_LOCK();
        v_scores.push_back(score);
        MUTEX_UNLOCK();
        count++;
    }

    if (count > DNN_OUTPUT_TENSOR_SIZE(detect_num)) {
        ERR_PRINTF("Invalid count index %u", count);
        return -1;
    }

    // Extract number of Detections
    uint8_t numOfDetections = (uint8_t) out_data[count];

    if (numOfDetections > totalDetections) {
         WARN_PRINTF("Unexpected value for numOfDetections: %d, setting it to %d",numOfDetections,totalDetections);
         numOfDetections = totalDetections;
    }

    objectdetection_output->num_of_detections_ = numOfDetections;
    MUTEX_LOCK();
    objectdetection_output->bboxes_ = v_bbox;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    objectdetection_output->scores_ = v_scores;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    objectdetection_output->classes_ = v_classes;
    MUTEX_UNLOCK();

    return 0;
}

void AnalyzerOd::analyzeDetectionOutput(AnalyzerOd::DetectionOutputTensor out_tensor, AnalyzerOd::DetectionData *output_objectdetection_data, AnalyzerOd::PPLParam param, uint64_t trace_id)
{
    uint8_t num_of_detections;
    uint8_t detections_above_threshold = 0;
    std::vector<Rect>    v_bbox;
    std::vector<float>   v_scores;
    std::vector<uint8_t> v_classes;
    std::vector<bool>    v_is_used_for_cropping;
    DetectionData objectdetection_data;

    // Extract number of detections
    num_of_detections = (uint8_t)out_tensor.num_of_detections_;
    for (uint8_t i = 0; i < num_of_detections; i++) {
        // Extract classes
        uint8_t class_index;
        MUTEX_LOCK();
        class_index = (uint8_t)out_tensor.classes_[i];
        MUTEX_UNLOCK();

        // Filter class
        if (class_index != BIRD_CLASS) {
            continue;
        }
        // Extract scores
        float score;
        MUTEX_LOCK();
        score = out_tensor.scores_[i];
        MUTEX_UNLOCK();
        
        // Filter Detections
        if (score < param.threshold_) {
            continue;
        } else {
            MUTEX_LOCK();
            v_classes.push_back(class_index);
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            v_scores.push_back(score);
            MUTEX_UNLOCK();

            // Extract bounding box co-ordinates
            Rect bbox;
            MUTEX_LOCK();
            bbox.m_xmin_ = (uint16_t)(round((out_tensor.bboxes_[i].x_min_) * (param.input_width_ - 1)));
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            bbox.m_ymin_ = (uint16_t)(round((out_tensor.bboxes_[i].y_min_) * (param.input_height_  - 1)));
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            bbox.m_xmax_ = (uint16_t)(round((out_tensor.bboxes_[i].x_max_) * (param.input_width_  - 1)));
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            bbox.m_ymax_ = (uint16_t)(round((out_tensor.bboxes_[i].y_max_) * (param.input_height_ - 1)));
            MUTEX_UNLOCK();
            MUTEX_LOCK();
            v_bbox.push_back(bbox);
            MUTEX_UNLOCK();

            MUTEX_LOCK();
            v_is_used_for_cropping.push_back(false);
            MUTEX_UNLOCK();

            detections_above_threshold++;
        }
    }

    objectdetection_data.num_of_detections_ = detections_above_threshold;
    MUTEX_LOCK();
    objectdetection_data.v_bbox_ = v_bbox;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    objectdetection_data.v_scores_ = v_scores;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    objectdetection_data.v_classes_ = v_classes;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    objectdetection_data.v_is_used_for_cropping_ = v_is_used_for_cropping;
    MUTEX_UNLOCK();

    if (objectdetection_data.num_of_detections_ > param.max_detections_) {
        objectdetection_data.num_of_detections_ = param.max_detections_;
        MUTEX_LOCK();
        objectdetection_data.v_bbox_.resize(param.max_detections_);
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        objectdetection_data.v_classes_.resize(param.max_detections_);
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        objectdetection_data.v_scores_.resize(param.max_detections_);
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        objectdetection_data.v_is_used_for_cropping_.resize(param.max_detections_);
        MUTEX_UNLOCK();
    }

    DBG_PRINTF("number of detections = %d", objectdetection_data.num_of_detections_);
    num_of_detections = objectdetection_data.num_of_detections_;

    // check the highest score and set is_used_for_cropping parameter to true
    if (num_of_detections > 0) {
        float   highest_score       = 0;
        uint8_t highest_score_index = 0;
        for (int i = 0; i < num_of_detections; i++) {
            MUTEX_LOCK();
            float score = objectdetection_data.v_scores_[i];
            MUTEX_UNLOCK();
            if (score > highest_score) {
                highest_score = score;
                highest_score_index = i;
            }
        }
        MUTEX_LOCK();
        objectdetection_data.v_is_used_for_cropping_.at(highest_score_index) = true;
        MUTEX_UNLOCK();
        DBG_PRINTF("The coordinates in the data with the highest score %f are used for Crop", highest_score);
    }

    output_objectdetection_data->num_of_detections_ = objectdetection_data.num_of_detections_;
    MUTEX_LOCK();
    output_objectdetection_data->v_bbox_ = objectdetection_data.v_bbox_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    output_objectdetection_data->v_scores_ = objectdetection_data.v_scores_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    output_objectdetection_data->v_classes_ = objectdetection_data.v_classes_;
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    output_objectdetection_data->v_is_used_for_cropping_ = objectdetection_data.v_is_used_for_cropping_;
    MUTEX_UNLOCK();
    output_objectdetection_data->trace_id_ = trace_id;

    for (int i = 0; i < num_of_detections; i++) {
        MUTEX_LOCK();
        uint16_t xmin = objectdetection_data.v_bbox_[i].m_xmin_;
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        uint16_t ymin = objectdetection_data.v_bbox_[i].m_ymin_;
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        uint16_t xmax = objectdetection_data.v_bbox_[i].m_xmax_;
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        uint16_t ymax = objectdetection_data.v_bbox_[i].m_ymax_;
        MUTEX_UNLOCK();
        DBG_PRINTF(
            "v_bbox[%d] :[x_min,y_min,x_max,y_max] = [%d,%d,%d,%d]",
            i,
            xmin,
            ymin,
            xmax,
            ymax
        );
        MUTEX_LOCK();
        float score = objectdetection_data.v_scores_[i];
        MUTEX_UNLOCK();
        DBG_PRINTF("scores[%d] = %f", i, score);
        MUTEX_LOCK();
        uint8_t class_indice = objectdetection_data.v_classes_[i];
        MUTEX_UNLOCK();
        DBG_PRINTF("class_indices[%d] = %d", i, class_indice);
        MUTEX_LOCK();
        bool used_for_cropping = objectdetection_data.v_is_used_for_cropping_[i];
        MUTEX_UNLOCK();
        DBG_PRINTF("is_used_for_cropping[%d] = %s", i, used_for_cropping ? "true" : "false");
    }
    DBG_PRINTF("trace_id = %llu", trace_id);

    return;
}

void AnalyzerOd::createSSDOutputFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const AnalyzerOd::DetectionData* detection_data) {
    std::vector<flatbuffers::Offset<SmartCamera::GeneralObject>> gdata_vector;
    uint8_t numOfDetections = detection_data->num_of_detections_;
    for (uint8_t i = 0; i < numOfDetections; i++) {
        DBG_PRINTF("left = %d, top = %d, right = %d, bottom = %d, class = %d, score = %f", detection_data->v_bbox_[i].m_xmin_, detection_data->v_bbox_[i].m_ymin_, detection_data->v_bbox_[i].m_xmax_, detection_data->v_bbox_[i].m_ymax_, detection_data->v_classes_[i], detection_data->v_scores_[i]);
        MUTEX_LOCK();
        auto bbox_data = SmartCamera::CreateBoundingBox2d(*builder, detection_data->v_bbox_[i].m_xmin_, \
            detection_data->v_bbox_[i].m_ymin_, \
            detection_data->v_bbox_[i].m_xmax_, \
            detection_data->v_bbox_[i].m_ymax_);
        MUTEX_UNLOCK();

        MUTEX_LOCK();
        auto general_data = SmartCamera::CreateGeneralObject(*builder, detection_data->v_classes_[i], SmartCamera::BoundingBox_BoundingBox2d, bbox_data.Union(), detection_data->v_scores_[i], detection_data->v_is_used_for_cropping_[i]);
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        gdata_vector.push_back(general_data);
        MUTEX_UNLOCK();
    }

    MUTEX_LOCK();
    auto v_od_data = builder->CreateVector(gdata_vector);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto od_data = SmartCamera::CreateObjectDetectionData(*builder, v_od_data);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto out_data = SmartCamera::CreateObjectDetectionTop(*builder, od_data, detection_data->trace_id_);
    MUTEX_UNLOCK();

    MUTEX_LOCK();
    builder->Finish(out_data);
    MUTEX_UNLOCK();

    return;
}

/////////////////////////////////
// AnalyzerIc
/////////////////////////////////

AnalyzerBase::ResultCode AnalyzerIc::ValidateParam(const void* param)
{
    DBG_PRINTF("AnalyzerIc::ValidateParam");
    AnalyzerBase::ScopeLock lock(this);

    // parse the json parameter
    JSON_Value *root_value;
    MUTEX_LOCK();
    root_value = json_parse_string((char *)param);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    JSON_Value_Type type = json_value_get_type(root_value);
    MUTEX_UNLOCK();
    if (type != JSONObject) {
        MUTEX_LOCK();
        json_value_free(root_value);
        MUTEX_UNLOCK();
        ERR_PRINTF("AnalyzerIc::ValidateParam: Invalid param");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    if (!validating_param_) {
        validating_param_ = new PPLParam;
    }
    AnalyzerBase::ResultCode ret = ClassificationParamInit(root_value, validating_param_);
    if (ret != AnalyzerBase::ResultCode::kOk) {
        ERR_PRINTF("AnalyzerIc::ValidateParam: Get json_parse Fail Err[%d]", ret);
        MUTEX_LOCK();
        json_value_free(root_value);
        MUTEX_UNLOCK();
        DoClearValidatingParam();
        return ret;
    }

    MUTEX_LOCK();
    json_value_free(root_value);
    MUTEX_UNLOCK();

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::SetValidatedParam(const void* param)
{
    DBG_PRINTF("AnalyzerIc::SetValidatedParam");
    AnalyzerBase::ScopeLock lock(this);

    if (!validating_param_) {
        return AnalyzerBase::ResultCode::kOtherError;   
    }

    // set ppl parameter
    param_ = *validating_param_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::ClearValidatingParam()
{
    DBG_PRINTF("AnalyzerIc::ClearValidatingParam");
    AnalyzerBase::ScopeLock lock(this);
    DoClearValidatingParam();
    return AnalyzerBase::ResultCode::kOk;
}

void AnalyzerIc::DoClearValidatingParam()
{
    if (validating_param_) {
        delete validating_param_;
        validating_param_ = nullptr;
    }
}

AnalyzerBase::ResultCode AnalyzerIc::Analyze(const float* p_data, uint32_t size, uint64_t trace_id)
{
    DBG_PRINTF("AnalyzerIc::Analyze");
    AnalyzerBase::ScopeLock lock(this);

    PPLParam ppl_parameter;
    AnalyzerIc::GetParam(ppl_parameter);

    if (p_data == nullptr) {
        ERR_PRINTF("AnalyzerIc::Analyze: Invalid param pdata=nullptr");
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    if (size != ppl_parameter.dnn_output_classes_) {
        ERR_PRINTF("AnalyzerIc::Analyze: Unexpected value for output tensor size %d",size);
        ERR_PRINTF("AnalyzerIc::Analyze: Expected value for output tensor size %d",ppl_parameter.dnn_output_classes_);
        return AnalyzerBase::ResultCode::kInvalidParam;
    }

    // call interface process
    ClassificationOutputTensor classification_output;
    createClassificationData(p_data, ppl_parameter.dnn_output_classes_, &classification_output);

    // call analyze process
    ClassificationData output_classification_data;
    analyzeClassificationOutput(classification_output, &output_classification_data, ppl_parameter, trace_id);

    data_ = output_classification_data;

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::Serialize(void** out_buf, uint32_t* size, const Allocator* allocator)
{
    DBG_PRINTF("AnalyzerIc::Serialize");
    AnalyzerBase::ScopeLock lock(this);

    ClassificationData classification_data;
    AnalyzerIc::GetAnalyzedData(classification_data);

    MUTEX_LOCK();
    flatbuffers::FlatBufferBuilder* builder = new flatbuffers::FlatBufferBuilder();
    MUTEX_UNLOCK();
    createClassificationFlatbuffer(builder, &classification_data);

    MUTEX_LOCK();
    uint8_t* buf_ptr = builder->GetBufferPointer();
    MUTEX_UNLOCK();
    if (buf_ptr == nullptr) {
        ERR_PRINTF("Error could not create Flatbuffer");
        MUTEX_LOCK();
        builder->Clear();
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        delete(builder);
        MUTEX_UNLOCK();
        return AnalyzerBase::ResultCode::kOtherError;
    }

    MUTEX_LOCK();
    uint32_t buf_size = builder->GetSize();
    MUTEX_UNLOCK();
    uint8_t *p_out_param = nullptr;
    p_out_param = (uint8_t *)allocator->Malloc(buf_size);
    if (p_out_param == nullptr) {
        ERR_PRINTF("malloc failed for creating flatbuffer, malloc size=%u", buf_size);
        MUTEX_LOCK();
        builder->Clear();
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        delete(builder);
        MUTEX_UNLOCK();
        return AnalyzerBase::ResultCode::kMemoryError;
    }
    DBG_PRINTF("p_out_param = %p", p_out_param);

    // Copy Data
    MUTEX_LOCK();
    memcpy(p_out_param, buf_ptr,buf_size);
    MUTEX_UNLOCK();
    *out_buf = p_out_param;
    *size = buf_size;

    // Clean up
    MUTEX_LOCK();
    builder->Clear();
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    delete(builder);
    MUTEX_UNLOCK();

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::GetNetworkId(uint32_t& network_id) const
{
    network_id = network_id_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::GetAnalyzedData(AnalyzerIc::ClassificationData& data) const
{
    data = data_;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::SetNetworkId(uint32_t network_id)
{
    network_id_ = network_id;
    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::ClassificationParamInit(JSON_Value *root_value, AnalyzerIc::PPLParam *p_cls_param)
{
    int ret;
    MUTEX_LOCK();
    ret = json_object_has_value(json_object(root_value),"models");
    MUTEX_UNLOCK();
    if (ret) {
        MUTEX_LOCK();
        JSON_Object* models = json_object_get_object(json_object(root_value), "models");
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        ret = json_object_has_value(models,"classification_bird");
        MUTEX_UNLOCK();
        if (ret) {
            MUTEX_LOCK();
            JSON_Object* classification_param = json_object_get_object(models, "classification_bird");
            MUTEX_UNLOCK();

            // Check network id
            MUTEX_LOCK();
            ret = json_object_has_value(classification_param,"network_id");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                const char* network_id_str = json_object_get_string(classification_param,"network_id");
                MUTEX_UNLOCK();
                if (network_id_str == nullptr) {
                    ERR_PRINTF("ClassificationParamInit: network_id is invalid");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
                MUTEX_LOCK();
                ret = strnlen(network_id_str, 7);
                MUTEX_UNLOCK();
                if (ret != NETWORK_ID_LEN) {
                    ERR_PRINTF("ClassificationParamInit: network_id must be six characters");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
                char* end;
                MUTEX_LOCK();
                long int network_id = strtol(network_id_str, &end, 16);
                MUTEX_UNLOCK();
                if (network_id == 0) {
                    ERR_PRINTF("ClassificationParamInit: network_id is invalid");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                } else {
                    SetNetworkId(network_id);
                    DBG_PRINTF("ClassificationParamInit network_id :%lx", network_id);
                }
            } else {
                ERR_PRINTF("ClassificationParamInit: json file does not have network_id");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }

            // Check param
            MUTEX_LOCK();
            ret = json_object_has_value(classification_param,"param");
            MUTEX_UNLOCK();
            if (ret) {
                MUTEX_LOCK();
                JSON_Object* param = json_object_get_object(classification_param, "param");
                MUTEX_UNLOCK();
                MUTEX_LOCK();
                ret = json_object_has_value(param,"dnn_output_classes");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t dnn_output_classes = json_object_get_number(param, "dnn_output_classes");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ClassificationParamInit dnn_output_classes: %d", dnn_output_classes);
                    p_cls_param->dnn_output_classes_ = dnn_output_classes;
                } else {
                    ERR_PRINTF("ClassificationParamInit: json file does not have dnn_output_classes");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }

                MUTEX_LOCK();
                ret = json_object_has_value(param,"max_predictions");
                MUTEX_UNLOCK();
                if (ret) {
                    MUTEX_LOCK();
                    uint16_t maxPredictions = (int)json_object_get_number(param, "max_predictions");
                    MUTEX_UNLOCK();
                    DBG_PRINTF("ClassificationParamInit max_predictions: %d", maxPredictions);

                    // check param
                    if (maxPredictions > p_cls_param->dnn_output_classes_) {
                        WARN_PRINTF("max_predictions > max number of classes, set to max number of classes");
                        p_cls_param->max_predictions_ = p_cls_param->dnn_output_classes_;
                    }
                    else {
                        p_cls_param->max_predictions_ = maxPredictions;
                    }
                } else {
                    ERR_PRINTF("ClassificationParamInit: json file does not have max_predictions");
                    return AnalyzerBase::ResultCode::kInvalidParam;
                }
            } else {
                ERR_PRINTF("ClassificationParamInit: json file does not have param");
                return AnalyzerBase::ResultCode::kInvalidParam;
            }
        } else {
            ERR_PRINTF("ClassificationParamInit: json file does not have classification_bird");
            return AnalyzerBase::ResultCode::kInvalidParam;
        }
    } else {
            ERR_PRINTF("ClassificationParamInit: json file does not have models");
            return AnalyzerBase::ResultCode::kInvalidParam;
    }

    return AnalyzerBase::ResultCode::kOk;
}

AnalyzerBase::ResultCode AnalyzerIc::GetParam(AnalyzerIc::PPLParam& param) const
{
    param = param_;
    return ResultCode::kOk;
}

void AnalyzerIc::createClassificationData(const float* data_body, uint32_t data_size, AnalyzerIc::ClassificationOutputTensor *classification_output)
{
    uint16_t size = data_size;
    for (uint16_t i = 0; i < size; i++) {
        float score = data_body[i];
        MUTEX_LOCK();
        classification_output->scores_.push_back(score);
        MUTEX_UNLOCK();
    }

    return;
}

void AnalyzerIc::analyzeClassificationOutput(AnalyzerIc::ClassificationOutputTensor out_tensor, AnalyzerIc::ClassificationData *output_classification_data, AnalyzerIc::PPLParam cls_param, uint64_t trace_id)
{
    uint16_t num_of_predictions;
    std::vector<ClassificationItem> class_data;

    // Extract number of predictions
    MUTEX_LOCK();
    num_of_predictions = out_tensor.scores_.size();
    MUTEX_UNLOCK();

    // Extract scores
    for (uint16_t i = 0; i < num_of_predictions; i++) {
        ClassificationItem item;
        item.index_ = i;
        MUTEX_LOCK();
        item.score_ = out_tensor.scores_[i];
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        class_data.push_back(item);
        MUTEX_UNLOCK();
    }

    MUTEX_LOCK();
    uint16_t num_of_classes = class_data.size();
    MUTEX_UNLOCK();
    if (num_of_classes > 0) {
        MUTEX_LOCK();
        std::stable_sort(class_data.begin(), class_data.end(), [](const ClassificationItem &left, const ClassificationItem &right) {
            return left.score_ > right.score_;
        });
        MUTEX_UNLOCK();
    }

    for (uint16_t i = 0; i < cls_param.max_predictions_; i++) {
        MUTEX_LOCK();
        output_classification_data->v_classItem_.push_back(class_data[i]);
        MUTEX_UNLOCK();
        DBG_PRINTF("Top[%d] = id: %d  score: %f", i, class_data[i].index_, class_data[i].score_);
    }

    output_classification_data->trace_id_ = trace_id;
    DBG_PRINTF("trace_id = %llu", trace_id);
    
    return;
}

void AnalyzerIc::createClassificationFlatbuffer(flatbuffers::FlatBufferBuilder* builder, const AnalyzerIc::ClassificationData *classificationData)
{
    std::vector<flatbuffers::Offset<SmartCamera::GeneralClassification>> gdata_vector;
    MUTEX_LOCK();
    uint16_t num_of_classes = classificationData->v_classItem_.size();
    MUTEX_UNLOCK();
    for (uint16_t i = 0; i < num_of_classes; i++) {
        MUTEX_LOCK();
        auto item = classificationData->v_classItem_[i];
        MUTEX_UNLOCK();
        DBG_PRINTF("class = %d, score = %f", item.index_,  item.score_);
        MUTEX_LOCK();
        auto general_data = SmartCamera::CreateGeneralClassification(*builder, item.index_, item.score_);
        MUTEX_UNLOCK();
        MUTEX_LOCK();
        gdata_vector.push_back(general_data);
        MUTEX_UNLOCK();
    }

    MUTEX_LOCK();
    auto v_bbox = builder->CreateVector(gdata_vector);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto class_data = SmartCamera::CreateClassificationData(*builder, v_bbox);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto out_data = SmartCamera::CreateClassificationTop(*builder, class_data, classificationData->trace_id_);
    MUTEX_UNLOCK();

    MUTEX_LOCK();
    builder->Finish(out_data);
    MUTEX_UNLOCK();

    return;
}
