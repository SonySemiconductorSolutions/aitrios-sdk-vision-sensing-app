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
#include <unistd.h>
#include <pthread.h>

#include "vision_app_public.h"
#include "analyzer_objectdetection.h"
#include "objectdetection_generated.h"

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define ERR_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "E [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define WARN_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "W [VisionAPP] ");fprintf(stderr, fmt, ##__VA_ARGS__);fprintf(stderr, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define INFO_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stdout, "I [VisionAPP] ");fprintf(stdout, fmt, ##__VA_ARGS__);fprintf(stdout, "\n");pthread_mutex_unlock(&g_libc_mutex);
#define DBG_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "D [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n");pthread_mutex_unlock(&g_libc_mutex);
#define VER_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "V [VisionAPP] "); printf( fmt, ##__VA_ARGS__); printf( "\n");pthread_mutex_unlock(&g_libc_mutex);

/* -------------------------------------------------------- */
/* static                                                   */
/* -------------------------------------------------------- */
static PPL_SsdParam s_ssd_param = {PPL_DNN_OUTPUT_DETECTIONS,PPL_DNN_OUTPUT_DETECTIONS, PPL_DEFAULT_THRESHOLD, PPL_SSD_INPUT_TENSOR_WIDTH, PPL_SSD_INPUT_TENSOR_HEIGHT}; //Set Max detections=10 Default Threshold= 0.3
static bool  s_is_evp_exit = false;
static bool  s_stream_stopped = true;
static int   s_private_data = 0;
static senscord_stream_t s_stream = NULL;

/* prevent libc func with multi thread */
pthread_mutex_t g_libc_mutex;

static const char *s_level_str[] = {"SENSCORD_LEVEL_UNDEFINED", "SENSCORD_LEVEL_FAIL", "SENSCORD_LEVEL_FATAL"};
static const char *s_cause_str[] = {"SENSCORD_ERROR_NONE",
                                "SENSCORD_ERROR_NOT_FOUND",
                                "SENSCORD_ERROR_INVALID_ARGUMENT",
                                "SENSCORD_ERROR_RESOURCE_EXHAUSTED",
                                "SENSCORD_ERROR_PERMISSION_DENIED",
                                "SENSCORD_ERROR_BUSY",
                                "SENSCORD_ERROR_TIMEOUT",
                                "SENSCORD_ERROR_CANCELLED",
                                "SENSCORD_ERROR_ABORTED",
                                "SENSCORD_ERROR_ALREADY_EXISTS",
                                "SENSCORD_ERROR_INVALID_OPERATION",
                                "SENSCORD_ERROR_OUT_OF_RANGE",
                                "SENSCORD_ERROR_DATA_LOSS",
                                "SENSCORD_ERROR_HARDWARE_ERROR",
                                "SENSCORD_ERROR_NOT_SUPPORTED",
                                "SENSCORD_ERROR_UNKNOWN"};

static void *evp_Thread(void *arg);
static void  ConfigurationCallback(const char *topic, const void *config, size_t config_len, void *private_data);
static void  SendDataDoneCallback(void *buf_addr, void *private_data, SessResult send_data_ret);
static void  PrintError(struct senscord_status_t status);
static void  FrameRecvCallback(senscord_stream_t stream, void *private_data);

/* -------------------------------------------------------- */
/* public function                                          */
/* -------------------------------------------------------- */
int main(int argc, char *argv[]) {
    senscord_core_t   core = NULL;
    pthread_t         evpthread_handle;
    const char*       stream_key = SENSCORD_STREAM_KEY_IMX500_IMAGE;
    struct senscord_status_t status;
    SessResult        sess_ret = kSessOther;
    int32_t           ret = -1;

    if (pthread_mutex_init(&g_libc_mutex, NULL) != 0) {
        printf("pthread_mutex_init failed libc_mutex");
        return -1;
    }

    if (setvbuf(stdout, NULL, _IOFBF, BUFSIZ) != 0) {
        ERR_PRINTF("fail setvbuf");
        return -1;
    }

    INFO_PRINTF("vision app objectdetection start\n");

    ret = pthread_create(&evpthread_handle, NULL, evp_Thread, NULL);
    if (ret != 0) {
        ERR_PRINTF("pthread_create failed for evp_Thread");
        return -1;
    }

    sess_ret = SessInit();
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessInit : sess_ret=%d", sess_ret);
        goto pthread_exit;
    }

    sess_ret = SessRegisterSendDataCallback((SessSendDataCallback)SendDataDoneCallback, &s_private_data);
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessRegisterSendDataCallback : sess_ret=%d", sess_ret);
        goto sess_exit;
    }

    ret = senscord_core_init(&core);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_init : ret=%d", ret);
        goto sess_unreg;
    }

    ret = senscord_core_open_stream(core, stream_key, &s_stream);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_open_stream : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
        goto exit;
    }

    ret = senscord_stream_start(s_stream);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_start : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
        goto close;
    }
    s_stream_stopped = false;

    ret = senscord_stream_register_frame_callback(s_stream, FrameRecvCallback, NULL);
    if (ret != 0) {
        ERR_PRINTF("senscord_stream_register_frame_callback : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
        goto stop;
    }

    struct input_tensor_dewarp_crop_property crop;
    ret = senscord_stream_get_property(s_stream, SENSCORD_PROPERTY_KEY_DEWARP_CROP, &crop, sizeof(struct input_tensor_dewarp_crop_property));
    if (ret != 0) {
        ERR_PRINTF("senscord_stream_get_property : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
        goto unreg;
    }
    INFO_PRINTF("Crop: [x=%d, y=%d, w=%d, h=%d]", crop.left, crop.top, crop.width, crop.height);

    while (!s_is_evp_exit) {
        senscord_frame_t    frame;
        senscord_channel_t  channel;
        struct senscord_raw_data_t raw_data;
        int32_t             timeout_msec = -1;
        uint32_t            channel_id_output_tensor = SENSCORD_CHANNEL_ID_OUTPUT_TENSOR;
        void               *p_out_buf = NULL;
        uint32_t            out_size = 0;

        ret = senscord_stream_get_frame(s_stream, &frame, timeout_msec);
        if (ret < 0) {
            ERR_PRINTF("senscord_stream_get_frame : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
            if (status.cause == SENSCORD_ERROR_TIMEOUT) {
                continue;
            }
            else {
                break;
            }
        }

        ret = senscord_frame_get_channel_from_channel_id(frame, channel_id_output_tensor, &channel);
        if (ret < 0) {
            ERR_PRINTF("senscord_frame_get_channel_from_channel_id : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
            break;
        }

        ret = senscord_channel_get_raw_data(channel, &raw_data);
        if (ret < 0) {
            ERR_PRINTF("senscord_channel_get_raw_data : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
            break;
        }
        INFO_PRINTF("raw_data.address:%p", raw_data.address);
        INFO_PRINTF("raw_data.size:%zu", raw_data.size);
        INFO_PRINTF("raw_data.timestamp:%llu", raw_data.timestamp);
        INFO_PRINTF("raw_data.type:%s", raw_data.type);

        ret = PPL_ObjectDetectionSsdAnalyze((float *)raw_data.address, raw_data.size/4, &p_out_buf, &out_size, s_ssd_param);
        INFO_PRINTF("Analyze done");

        if ((EPPL_RESULT_CODE)ret != E_PPL_OK) {
            ERR_PRINTF("PPL_ObjectDetectionSsdAnalyze : ret=%d", ret);
        }
        else {
            if (SESS_SEND_MAX_SIZE < out_size) {
                DBG_PRINTF("SessSendData : size=%d", out_size);

                pthread_mutex_lock(&g_libc_mutex);
                SessFree(p_out_buf);
                pthread_mutex_unlock(&g_libc_mutex);

                goto release;
            }

            sess_ret = SessSendData(p_out_buf, out_size, raw_data.timestamp);
            if (sess_ret == kSessOK) {
                /* Do Nothing */
                DBG_PRINTF("SessSend success");
            }
            else if (sess_ret == kSessNotStreaming) {
                DBG_PRINTF("camera not streaming : sess_ret=%d", sess_ret);
            }
            else {
                ERR_PRINTF("SessSendData : sess_ret=%d", sess_ret);
                break;
            }
        }

release:
        ret = senscord_stream_release_frame(s_stream, frame);
        if (ret < 0) {
            ERR_PRINTF("senscord_stream_release_frame : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
            break;
        }
    }

unreg:
    ret = senscord_stream_unregister_frame_callback(s_stream);
    if (ret < 0) {
        ERR_PRINTF("senscord_stream_unregister_frame_callback : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
    }

stop:
    if (s_stream_stopped == false) {
        ret = senscord_stream_stop(s_stream);
        if (ret < 0) {
            ERR_PRINTF("senscord_stream_stop : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
        }
        else {
            s_stream_stopped = true;
        }
    }

close:
    ret = senscord_core_close_stream(core, s_stream);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_close_stream : ret=%d", ret);
        senscord_get_last_error(&status);
        PrintError(status);
    }

exit:
    ret = senscord_core_exit(core);
    if (ret < 0) {
        ERR_PRINTF("senscord_core_exit : ret=%d", ret);
    }

sess_unreg:
    sess_ret = SessUnregisterSendDataCallback();
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessUnregisterSendDataCallback : sess_ret=%d", sess_ret);
    }

sess_exit:
    sess_ret = SessExit();
    if (sess_ret != kSessOK) {
        ERR_PRINTF("SessExit : sess_ret=%d", sess_ret);
    }

pthread_exit:
    s_is_evp_exit = true;

    ret = pthread_join(evpthread_handle, NULL);
    if (ret != 0) {
        ERR_PRINTF("pthread_join error");
    }

    pthread_mutex_destroy(&g_libc_mutex);
    return 0;
}

static void *evp_Thread(void *arg) {

    struct EVP_client* handle = EVP_initialize();
    INFO_PRINTF("EVP client handle:%p\n", handle);
    EVP_RESULT evp_ret = EVP_OK;
    evp_ret = EVP_setConfigurationCallback(handle, (EVP_CONFIGURATION_CALLBACK)ConfigurationCallback, &s_private_data);
    INFO_PRINTF("EVP_setConfigurationCallback evp_ret:%d\n", evp_ret);

    while (1) {
        if (s_is_evp_exit == true) {
            ERR_PRINTF("pthread_exit");
            pthread_exit(NULL);
            break;
        }

        int32_t timeout_msec = 1000;
        evp_ret = EVP_processEvent(handle, timeout_msec);
        if (evp_ret == EVP_SHOULDEXIT) {
            INFO_PRINTF("Should exit vision app");
            s_is_evp_exit = true;
            if ((s_stream != NULL) && (s_stream_stopped == false)) {
                int32_t ret = senscord_stream_stop(s_stream);
                if (ret < 0) {
                    ERR_PRINTF("senscord_stream_stop : ret=%d", ret);
                    struct senscord_status_t status;
                    senscord_get_last_error(&status);
                    PrintError(status);
                    break;
                }
                s_stream_stopped = true;
            }
            break;
        }
        else if (evp_ret == EVP_TIMEDOUT) {
            /* Do Nothing */
        }
        else if (evp_ret) {
            ERR_PRINTF("Failed to EVP_processEvent:%d\n", evp_ret);
        }
        else {
            /* Do Nothing */
        }
    }

    return NULL;
}

static void FrameRecvCallback(senscord_stream_t stream, void *private_data)
{
    INFO_PRINTF("Frame recv. stream:%p, private_data:%p", stream, private_data);
    return;
}

static void ConfigurationCallback(const char *topic, const void *config, size_t configlen, void *userData) {

    DBG_PRINTF("%s", __func__);

    if ((char *)config == NULL) {
        ERR_PRINTF("Invalid param : config=NULL");
        return;
    }
    INFO_PRINTF("%s topic:%s\nconfig:%s\nconfig_len:%zu\nuserData:%p\n", __func__, topic, (char*)config, configlen, userData);

    pthread_mutex_lock(&g_libc_mutex);
    int str_ret = strcmp((char *)config, "");
    pthread_mutex_unlock(&g_libc_mutex);
    if (str_ret == 0) {
        INFO_PRINTF("ConfigurationCallback: config is empty.");
        INFO_PRINTF("dnn_output_detections: %d  max_detections: %d threshold: %f input_width: %d input_height: %d", \
            s_ssd_param.dnnOutputDetections, s_ssd_param.maxDetections, s_ssd_param.threshold, s_ssd_param.inputWidth, s_ssd_param.inputHeight);
        return;
    }

    // parse the json parameter
    JSON_Value *root_value;

    pthread_mutex_lock(&g_libc_mutex);
    root_value = json_parse_string((char *)config);
    pthread_mutex_unlock(&g_libc_mutex);

    pthread_mutex_lock(&g_libc_mutex);
    JSON_Value_Type type = json_value_get_type(root_value);
    pthread_mutex_unlock(&g_libc_mutex);

    if (type != JSONObject) {
        pthread_mutex_lock(&g_libc_mutex);
        json_value_free(root_value);
        pthread_mutex_unlock(&g_libc_mutex);

        ERR_PRINTF("Invalid param");
        return;
    }

    EPPL_RESULT_CODE ret = json_parse(root_value, &s_ssd_param);

    if (ret != E_PPL_OK) {
        ERR_PRINTF("ConfigurationCallback: Get json_parse Fail Err[%d]", ret);
        pthread_mutex_lock(&g_libc_mutex);
        json_value_free(root_value);
        pthread_mutex_unlock(&g_libc_mutex);
        return;
    }
    pthread_mutex_lock(&g_libc_mutex);
    json_value_free(root_value);
    pthread_mutex_unlock(&g_libc_mutex);

    return;
}

static void SendDataDoneCallback(void *buf_addr, void *private_data, SessResult send_data_ret)
{
    if (send_data_ret == kSessOK) {
        if (buf_addr != NULL) {
            pthread_mutex_lock(&g_libc_mutex);
            SessFree(buf_addr);
            pthread_mutex_unlock(&g_libc_mutex);
        }
    }
    else {
        ERR_PRINTF("SendDataDoneCallback : send_data_ret:%d", send_data_ret);
        if (buf_addr != NULL) {
            pthread_mutex_lock(&g_libc_mutex);
            SessFree(buf_addr);
            pthread_mutex_unlock(&g_libc_mutex);
        }
    }
}

static void PrintError(struct senscord_status_t status)
{
    DBG_PRINTF("status:\n - level  : %s\n - cause  : %s\n - message: %s\n - block  : %s",
        s_level_str[status.level], s_cause_str[status.cause], status.message, status.block);
}
