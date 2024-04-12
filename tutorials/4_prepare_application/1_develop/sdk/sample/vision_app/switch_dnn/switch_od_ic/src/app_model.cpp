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

#include "app_model.h"
#include "defines.h"

AppModel::AppModel(AnalyzerOd* analyzer_od,
                   AnalyzerIc* analyzer_ic) : analyzer_od_(analyzer_od), analyzer_ic_(analyzer_ic) {
}

AppModel::~AppModel() {
    ClearNextCropProperty();
    ClearNextDnnProperty();
}

uint64_t AppModel::TraceId() const {
    return trace_id_;
}

void AppModel::SetTraceId(uint64_t trace_id) {
    trace_id_ = trace_id;
}

void AppModel::ClearTraceId() {
    trace_id_ = 0;
}

const senscord_dnn_property* AppModel::NextDnnProperty() const {
    return next_dnn_property_;
}

void AppModel::SetNextDnnProperty(struct senscord_dnn_property* property) {
    if (property != nullptr) {
        INFO_PRINTF("SetNextDnnProperty network_id:%x network_ordinal:%d", property->network_id, property->network_ordinal);
    } else {
        WARN_PRINTF("SetNextDnnProperty null");
    }
    if (next_dnn_property_ != nullptr) {
        WARN_PRINTF("SetNextDnnProperty next_dnn_property_ not null");
        ClearNextDnnProperty();
    }
    next_dnn_property_ = property;
}

void AppModel::ClearNextDnnProperty() {
    delete next_dnn_property_;
    next_dnn_property_ = nullptr;
}

input_tensor_dewarp_crop_property* AppModel::NextCropProperty() const {
    return next_crop_property_;
}

void AppModel::SetNextCropProperty(struct input_tensor_dewarp_crop_property* property) {
    if (property != nullptr) {
        INFO_PRINTF("SetNextCropProperty left:%d top:%d width:%d height:%d", property->left, property->top, property->width, property->height);
    } else {
        WARN_PRINTF("SetNextCropProperty null");
    }
    if (next_crop_property_ != nullptr) {
        WARN_PRINTF("SetNextCropProperty next_crop_property_ not null");
        ClearNextCropProperty();
    }
    next_crop_property_ = property;
}

void AppModel::ClearNextCropProperty() {
    delete next_crop_property_;
    next_crop_property_ = nullptr;
}
