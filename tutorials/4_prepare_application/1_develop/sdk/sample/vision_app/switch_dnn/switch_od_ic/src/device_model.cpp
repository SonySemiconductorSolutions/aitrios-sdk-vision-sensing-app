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

#include "device_model.h"
#include "defines.h"

DeviceModel::DeviceModel() {}

DeviceModel::~DeviceModel() {}

senscord_frame_t& DeviceModel::Frame() {
    return frame_;
}

senscord_channel_t& DeviceModel::Channel() {
    return channel_;
}

senscord_dnn_property& DeviceModel::DnnStream() {
    return dnn_stream_;
}

senscord_dnn_property& DeviceModel::DnnChannel() {
    return dnn_channel_;
}

input_tensor_dewarp_crop_property& DeviceModel::CropStream() {
    return crop_stream_;
}

input_tensor_dewarp_crop_property& DeviceModel::CropChannel() {
    return crop_channel_;
}
