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

#ifndef _APP_MODEL_H_
#define _APP_MODEL_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

/**
 * @class AppModel
 * @brief App model class. The class manages application's data used across some app states.
 */
class AppModel {
public:
    /**
     * @brief Constructor
     */
    AppModel(AnalyzerOd* analyzer_od,
             AnalyzerIc* analyzer_ic);

    /**
     * @brief Destructor
     */
    virtual ~AppModel();

    /**
     * @brief Get trace id
     * @return trace id
     */
    uint64_t TraceId() const;

    /**
     * @brief Set trace id
     * @param [in] trace_id trace id
     */
    void SetTraceId(uint64_t trace_id);

    /**
     * @brief Clear trace id
     */
    void ClearTraceId();

    /**
     * @brief Get next dnn property
     * @return senscord_dnn_property
     */
    const senscord_dnn_property* NextDnnProperty() const;

    /**
     * @brief Set next dnn property
     * @param [in] senscord_dnn_property dnn property
     */
    void SetNextDnnProperty(struct senscord_dnn_property* property);

    /**
     * @brief Clear next dnn property
     */
    void ClearNextDnnProperty();

    /**
     * @brief Get next crop property
     * @return input_tensor_dewarp_crop_property
     */
    input_tensor_dewarp_crop_property* NextCropProperty() const;

    /**
     * @brief Set next crop property
     * @param [in] input_tensor_dewarp_crop_property crop property
     */
    void SetNextCropProperty(struct input_tensor_dewarp_crop_property* property);

    /**
     * @brief Clear next crop property
     */
    void ClearNextCropProperty();

private:
    AnalyzerOd* analyzer_od_;
    AnalyzerIc* analyzer_ic_;

private:
    uint64_t trace_id_ = 0;
    struct senscord_dnn_property* next_dnn_property_ = nullptr;
    struct input_tensor_dewarp_crop_property* next_crop_property_ = nullptr;
};

#endif // _APP_MODEL_H_
