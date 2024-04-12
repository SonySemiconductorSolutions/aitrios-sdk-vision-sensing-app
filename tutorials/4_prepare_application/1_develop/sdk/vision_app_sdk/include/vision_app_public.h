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

#ifndef _VISION_APP_PUBLIC_H_
#define _VISION_APP_PUBLIC_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define SENSCORD_STREAM_KEY_IMX500_IMAGE "imx500_image"
#define SENSCORD_CHANNEL_ID_OUTPUT_TENSOR (0x80000001)
#define SENSCORD_RAW_DATA_TYPE_OUTPUT_TENSOR "OutputTensor"
#define SENSCORD_PROPERTY_KEY_DEWARP_CROP "devicesdk_imx500_input_tensor_dewarp_crop_property"
#define SENSCORD_PROPERTY_KEY_DNN         "devicesdk_imx500_dnn_property"

#define SESS_SEND_MAX_SIZE 90112 /**< 88KB */

struct input_tensor_dewarp_crop_property {
  uint16_t left;       /**< start xpoint */
  uint16_t top;        /**< start ypoint */
  uint16_t width;      /**< width */
  uint16_t height;     /**< height */
};

struct senscord_dnn_property {
  uint32_t network_id; /**< network id */
  uint8_t network_ordinal; /**< network ordinal */
};

typedef void* senscord_core_t;
typedef void* senscord_stream_t;
typedef void* senscord_frame_t;
typedef void* senscord_channel_t;
typedef void (*senscord_frame_received_callback)(senscord_stream_t stream, void* private_data);

struct senscord_raw_data_t {
  void* address;       /**< virtual address */
  size_t size;         /**< data size */
  char* type;    /**< data type*/
  uint64_t timestamp;  /**< nanoseconds timestamp captured by the device */
};

/* enum */
/**
 * @brief The type to represent either a success or an error.
 * Used as the return value of several of the functions in this SDK.
 */
typedef enum {
	/**
	 * At least one event has been processed.
	 */
	EVP_OK = 0,

	/**
	 * The module instance is requested to stop.
	 * It should exit performing cleanup as soon as possible.
	 */
	EVP_SHOULDEXIT = 1,

	/**
	 * The specified period has ellapsed without any events.
	 */
	EVP_TIMEDOUT = 2,

	/**
	 * An error occurred.
	 */
	EVP_ERROR = 3,

	/**
	 * Invalid parameter.
	 */
	EVP_INVAL = 4,

	/**
	 * Memory allocation failed.
	 */
	EVP_NOMEM = 5,

	/**
	 * Too big payload.
	 */
	EVP_TOOBIG = 6,

	/**
	 * Failure because of temporary conditions.
	 */
	EVP_AGAIN = 7,

	/**
	 * Protocol error when communicating with the agent.
	 */
	EVP_AGENT_PROTOCOL_ERROR = 8,

	/**
	 * The request failed bacause of conflicting existing entries.
	 */
	EVP_EXIST = 9,

	/**
	 * Invalid address was detected.
	 *
	 * Note: An application should not rely on such a detection.
	 * It's the responsibility of applications to always specify
	 * vaild addresses.
	 */
	EVP_FAULT = 10,
} EVP_RESULT;

typedef enum {
	kSessOK = 0,
	kSessOther,
	kSessInvalidOperation,
	kSessInvalidParam,
	kSessNotStreaming,
} SessResult;

enum senscord_error_level_t {
  	SENSCORD_LEVEL_UNDEFINED = 0,
  	SENSCORD_LEVEL_FAIL,
  	SENSCORD_LEVEL_FATAL,
};

enum senscord_error_cause_t {
	SENSCORD_ERROR_NONE = 0,
	SENSCORD_ERROR_NOT_FOUND,
	SENSCORD_ERROR_INVALID_ARGUMENT,
	SENSCORD_ERROR_RESOURCE_EXHAUSTED,
	SENSCORD_ERROR_PERMISSION_DENIED,
	SENSCORD_ERROR_BUSY,
	SENSCORD_ERROR_TIMEOUT,
	SENSCORD_ERROR_CANCELLED,
	SENSCORD_ERROR_ABORTED,
	SENSCORD_ERROR_ALREADY_EXISTS,
	SENSCORD_ERROR_INVALID_OPERATION,
	SENSCORD_ERROR_OUT_OF_RANGE,
	SENSCORD_ERROR_DATA_LOSS,
	SENSCORD_ERROR_HARDWARE_ERROR,
	SENSCORD_ERROR_NOT_SUPPORTED,
	SENSCORD_ERROR_UNKNOWN,
};

struct senscord_status_t {
	enum senscord_error_level_t level;  /* level of the error occurred */
	enum senscord_error_cause_t cause;  /* cause of the error occurred */
	const char* message;                /* error message */
	const char* block;                  /* internal block from where the error has occurred */
};

typedef void (*SessSendDataCallback)(void *data, void *private_data, SessResult send_data_ret);
typedef void (*EVP_CONFIGURATION_CALLBACK)(const char *topic, const void *config, size_t configlen, void *userData);

int32_t senscord_core_init(senscord_core_t* core);
int32_t senscord_core_exit(senscord_core_t core);
int32_t senscord_core_open_stream(senscord_core_t core, const char* stream_key, senscord_stream_t* stream);
int32_t senscord_core_close_stream(senscord_core_t core, senscord_stream_t stream);
int32_t senscord_stream_start(senscord_stream_t stream);
int32_t senscord_stream_stop(senscord_stream_t stream); 
int32_t senscord_stream_get_frame(senscord_stream_t stream, senscord_frame_t* frame, int32_t timeout_msec);
int32_t senscord_stream_release_frame(senscord_stream_t stream, senscord_frame_t frame);
int32_t senscord_stream_get_property(senscord_stream_t stream, const char* property_key, void* value, size_t value_size);
int32_t senscord_stream_set_property(senscord_stream_t stream, const char* property_key, const void* value, size_t value_size);
int32_t senscord_stream_register_frame_callback(senscord_stream_t stream, const senscord_frame_received_callback callback, void* private_data);
int32_t senscord_stream_unregister_frame_callback(senscord_stream_t stream);
int32_t senscord_frame_get_channel_from_channel_id(senscord_frame_t frame, uint32_t channel_id, senscord_channel_t* channel);
int32_t senscord_channel_get_raw_data(senscord_channel_t channel, struct senscord_raw_data_t* raw_data);
int32_t senscord_channel_get_property(senscord_channel_t channel, const char* property_key, void* value, size_t value_size);
int32_t senscord_get_last_error(struct senscord_status_t* status);

SessResult SessInit(void);
SessResult SessExit(void);
SessResult SessSendData(const void *data, size_t size, uint64_t timestamp); // timestamp: nanoseconds
inline void* SessMalloc(size_t size)
{
    return malloc(size);
}
inline void SessFree(void *buf)
{
    return free(buf);
}

SessResult SessRegisterSendDataCallback(SessSendDataCallback cb, void *private_data);
SessResult SessUnregisterSendDataCallback(void);

struct EVP_client* EVP_initialize(void);
EVP_RESULT EVP_setConfigurationCallback(struct EVP_client* handle, EVP_CONFIGURATION_CALLBACK cb, void* private_data);
EVP_RESULT EVP_processEvent(struct EVP_client* handle, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
