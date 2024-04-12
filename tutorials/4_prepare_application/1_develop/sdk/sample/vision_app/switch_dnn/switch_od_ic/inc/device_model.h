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

#ifndef _DEVICE_MODEL_H_
#define _DEVICE_MODEL_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "vision_app_public.h"

/**
 * @class DeviceModel
 * @brief Device model class. The class manages device's data used across some app states.
 */
class DeviceModel {
public:
    /**
     * @brief Constructor
     */
    DeviceModel();

    /**
     * @brief Destructor
     */
    virtual ~DeviceModel();

    /**
     * @brief Get frame
     * @return senscord_frame_t
     */
    senscord_frame_t& Frame();

    /**
     * @brief Get channel
     * @return senscord_channel_t
     */
    senscord_channel_t& Channel();

    /**
     * @brief Get dnn stream property
     * @return senscord_dnn_property
     */
    senscord_dnn_property& DnnStream();

    /**
     * @brief Get dnn channel property
     * @return senscord_dnn_property
     */
    senscord_dnn_property& DnnChannel();

    /**
     * @brief Get crop stream property
     * @return input_tensor_dewarp_crop_property
     */
    input_tensor_dewarp_crop_property& CropStream();

    /**
     * @brief Get crop channel property
     * @return input_tensor_dewarp_crop_property
     */
    input_tensor_dewarp_crop_property& CropChannel();

private:
    senscord_dnn_property dnn_stream_ = { 0, 0 };
    senscord_dnn_property dnn_channel_ = { 0, 0 };
    input_tensor_dewarp_crop_property crop_stream_ = { 0, 0, 0, 0 };
    input_tensor_dewarp_crop_property crop_channel_ = { 0, 0, 0, 0 };

private:
    senscord_frame_t frame_;
    senscord_channel_t channel_;
};

#endif // _DEVICE_MODEL_H_
