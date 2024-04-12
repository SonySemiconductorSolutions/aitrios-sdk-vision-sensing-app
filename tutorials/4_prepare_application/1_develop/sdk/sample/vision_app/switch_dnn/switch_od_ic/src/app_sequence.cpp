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

#include "app_sequence.h"
#include "defines.h"

/* -------------------------------------------------------- */
/* define                                                   */
/* -------------------------------------------------------- */
#define FATAL_ERROR_LOG_USLEEP_INTERVAL (100000)
#define FATAL_ERROR_LOG_COUNTER (50)
#define INITIALIZE_USLEEP_INTERVAL (100000)
#define WAIT_DATA_SEND_USLEEP_INTERVAL (100000)

AppSequence::AppSequence() : fatal_error_(false) {
    evp_ = new EvpController();
    evp_thread_ = new EvpThreadController();
    sess_allocator_ = new SessAllocator();
    sess_ = new SessController();
    senscord_ = new SenscordController();

    device_model_ = new DeviceModel();

    analyzer_com_ = new AnalyzerCommon();
    analyzer_od_  = new AnalyzerOd();
    analyzer_ic_  = new AnalyzerIc();

    app_model_ = new AppModel(analyzer_od_, analyzer_ic_);
}

AppSequence::~AppSequence() {
    delete app_model_;
    app_model_ = nullptr;
    delete analyzer_ic_;
    analyzer_ic_ = nullptr;
    delete analyzer_od_;
    analyzer_od_ = nullptr;
    delete analyzer_com_;
    analyzer_com_ = nullptr;
    delete device_model_;
    device_model_ = nullptr;
    delete senscord_;
    senscord_ = nullptr;
    delete sess_;
    sess_ = nullptr;
    delete sess_allocator_;
    sess_allocator_ = nullptr;
    delete evp_thread_;
    evp_thread_ = nullptr;
    delete evp_;
    evp_ = nullptr;
}

int32_t AppSequence::Initialize() {
    VER_PRINTF("[SEQ] Initialize start");
    MUTEX_LOCK();
    analyzers_ = {analyzer_com_, analyzer_od_, analyzer_ic_};
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto callback_validate = std::bind(&AppSequence::ValidateParam, this, std::placeholders::_1);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto callback_clear_validating_param = std::bind(&AppSequence::ClearValidatingParam, this);
    MUTEX_UNLOCK();
    MUTEX_LOCK();
    auto callback_set_validated_param = std::bind(&AppSequence::SetValidatedParam, this, std::placeholders::_1);
    MUTEX_UNLOCK();
    // initialize EVP
    int32_t ret = evp_->Initialize(callback_validate, callback_clear_validating_param, callback_set_validated_param);
    if (ret != 0) {
        ERR_PRINTF("[SEQ] Initialize evp error");
        return -1;
    }

    // start EVP thread
    ret = evp_thread_->Start(evp_, senscord_);
    if (ret != 0) {
        ERR_PRINTF("[SEQ] Initialize evp thread error");
        return -1;
    }

    // initialize Sess
    ret = sess_->Initialize(sess_allocator_);
    if (ret != 0) {
        ERR_PRINTF("[SEQ] Initialize sess error");
        return -1;
    }

    // initialize SensCord
    ret = senscord_->Open();
    if (ret != 0) {
        ERR_PRINTF("[SEQ] Initialize senscord error");
        return -1;
    }

    // Wait for first ConfigurationCallback called and succeeded to parse PPL Parameter
    // evp_->WaitSetParam();
    while (!evp_thread_->IsThreadTerminated()) {
        if (evp_->HasParam()) {
            break;
        }

        usleep(INITIALIZE_USLEEP_INTERVAL);
    }

    VER_PRINTF("[SEQ] Initialize end");
    return 0;
}

int32_t AppSequence::MainLoop() {
    VER_PRINTF("[SEQ] MainLoop start");
    AppContext context = AppContext(evp_, evp_thread_, sess_, sess_allocator_, senscord_,
                                    analyzer_com_, analyzer_od_, analyzer_ic_,
                                    device_model_, app_model_);

    while (!evp_thread_->IsThreadTerminated()) {
        VER_PRINTF("[SEQ] MainLoop");

        // Get frame from the device.
        AppState::SequenceStatus status = context.GetFrame();
        if (status != AppState::SequenceStatus::kSuccess) {
            if (status == AppState::SequenceStatus::kContinue) {
                // Senscord API returns fail error
                VER_PRINTF("[SEQ] GetFrame continue");
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] GetFrame error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Verify that expected DNN Model is set to the device.
        status = context.VerifyDnnStream();
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // OD model is not set at the start of inference
                // or Senscord API returns fail error
                VER_PRINTF("[SEQ] VerifyDnnStream continue");
                continue;
            } else if (status == AppState::SequenceStatus::kRetrySetProperty) {
                // previous set dnn failed or restart inference, so retry set dnn
                VER_PRINTF("[SEQ] VerifyDnnStream retry set dnn");
                status = context.RetrySetDnn();
                ReleaseFrameForContinueOrBreak(context, status);
                if (status == AppState::SequenceStatus::kErrorFatal) {
                    // Senscord API returns fatal error
                    ERR_PRINTF("[SEQ] RetrySetDnn error fatal");
                    fatal_error_ = true;
                    break;
                }
                if (status == AppState::SequenceStatus::kErrorChangeToOd) {
                    // Dnn setting to set to device is empty
                    VER_PRINTF("[SEQ] RetrySetDnn change to od");
                    context.ChangeState(SequenceState::kOd);
                }
                continue;
            } else if (status == AppState::SequenceStatus::kErrorChangeToOd) {
                // Unexpected model is set in device, change model to od
                VER_PRINTF("[SEQ] VerifyDnnStream change to od");
                context.ChangeState(SequenceState::kOd);
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] VerifyDnnStream error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Get channel from the frame.
        status = context.GetChannel();
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Senscord API returns fail error
                WARN_PRINTF("[SEQ] GetChannel continue");
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] GetChannel error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Verify DNN Model is set to the channel.
        status = context.VerifyDnnChannel();
        if (status == AppState::SequenceStatus::kSuccessCompleted) {
            // Dnn settings verified successfully
            SequenceState next_state = context.NextState();
            context.ChangeState(next_state);
        } else if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Continue until dnn model settings are reflected
                // or OD model is not set at the start of inference
                // or Senscord API returns fail error
                VER_PRINTF("[SEQ] VerifyDnnChannel continue");
                continue;
            } else if (status == AppState::SequenceStatus::kErrorChangeToOd) {
                // Unexpected model is set in device, change model to od
                VER_PRINTF("[SEQ] VerifyDnnChannel change to od");
                context.ChangeState(SequenceState::kOd);
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] VerifyDnnChannel error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Get crop property from the device and channel.
        status = context.GetCropProperty();
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Senscord API returns fail error
                WARN_PRINTF("[SEQ] GetCropProperty continue");
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] GetCropProperty error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Set crop property to the device.
        status = context.SetCrop();
        if (status == AppState::SequenceStatus::kSuccessCompleted) {
            // SetCrop executed and crop settings will be verified in next state
            context.ReleaseFrame();
            SequenceState next_state = context.NextState();
            context.ChangeState(next_state);
            continue;
        } else if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] SetCrop error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Check crop property is set to the device and channel.
        status = context.VerifyCrop();
        if (status == AppState::SequenceStatus::kSuccessCompleted) {
            // Crop settings verified successfully
            VER_PRINTF("[SEQ] VerifyCrop change to ic");
            SequenceState next_state = context.NextState();
            context.ChangeState(next_state);
        } else if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Continue until crop settings are reflected
                VER_PRINTF("[SEQ] VerifyCrop continue");
                continue;
            } else if (status == AppState::SequenceStatus::kRetrySetProperty) {
                // Previous set crop failed, so retry set crop
                VER_PRINTF("[SEQ] VerifyCrop retry set crop");
                status = context.RetrySetCrop();
                ReleaseFrameForContinueOrBreak(context, status);
                if (status == AppState::SequenceStatus::kErrorFatal) {
                    // Senscord API returns fatal error
                    ERR_PRINTF("[SEQ] RetrySetCrop error fatal");
                    fatal_error_ = true;
                    break;
                }
                if (status == AppState::SequenceStatus::kErrorChangeToOd) {
                    // Crop setting to verify is empty
                    VER_PRINTF("[SEQ] RetrySetCrop change to od");
                    context.ChangeState(SequenceState::kOd);
                }
                continue;
            } else if (status == AppState::SequenceStatus::kErrorChangeToOd) {
                // Crop setting to verify is empty
                VER_PRINTF("[SEQ] VerifyCrop change to od");
                context.ChangeState(SequenceState::kOd);
                continue;
            }
        }

        struct senscord_raw_data_t raw_data;

        // Get raw data from the channel.
        status = context.GetRawData(raw_data);
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Senscord API returns fail error
                WARN_PRINTF("[SEQ] GetRawData continue");
                continue;
            } else if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] GetRawData error fatal");
                fatal_error_ = true;
                break;
            }
        }

        // Analyze data.
        status = context.AnalyzeData(raw_data);
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Invalid PPL Parameter
                VER_PRINTF("[SEQ] AnalyzeData continue");
                continue;
            } else if (status == AppState::SequenceStatus::kError) {
                // Analyze failed
                ERR_PRINTF("[SEQ] AnalyzeData error");
                fatal_error_ = true;
                break;
            }
        }

        // Save the coordinates detected by OD as Crop settings for the IC.
        status = context.SaveCrop();
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kContinue) {
                // Invalid crop value
                VER_PRINTF("[SEQ] SaveCrop continue");
                continue;
            }
        }

        void*    p_out_buf = nullptr;
        uint32_t out_size = 0;

        // Serialize analyzed data.
        status = context.SerializeData(&p_out_buf, out_size);
        if (status != AppState::SequenceStatus::kSuccess) {
            ReleaseFrameForContinueOrBreak(context, status);
            if (status == AppState::SequenceStatus::kError) {
                // Serialize failed
                ERR_PRINTF("[SEQ] SerializeData error");
                fatal_error_ = true;
                break;
            }
        }

        // Send serialized data.
        status = context.SendSerializedData(p_out_buf, out_size, raw_data.timestamp);
        context.ReleaseFrame();
        if (status != AppState::SequenceStatus::kSuccess) {
            if (status == AppState::SequenceStatus::kContinue) {
                // Camera is not streaming
                // or data size is too large
                VER_PRINTF("[SEQ] SendSerializedData continue");
                continue;
            } else if (status == AppState::SequenceStatus::kError) {
                // Send data failed
                ERR_PRINTF("[SEQ] SendSerializedData error");
                fatal_error_ = true;
                break;
            }
        } else {
            // Wait for complete to send data.
            bool wait_cancelled = false;
            while (sess_->SendingCount() > 0) {
                if (!evp_thread_->IsThreadTerminated()) {
                    usleep(WAIT_DATA_SEND_USLEEP_INTERVAL);
                } else {
                    // break to exit application
                    VER_PRINTF("[SEQ] wait data send break");
                    wait_cancelled = true;
                    break;
                }
            }
            if (wait_cancelled) {
                // break to exit application
                VER_PRINTF("[SEQ] wait data send cancelled");
                break;
            }
        }

        // Set DNN Model to the device.
        status = context.SetDnn();
        if (status == AppState::SequenceStatus::kSuccessCompleted) {
            // SetDnn executed and dnn settings will be verified in next state
            SequenceState next_state = context.NextState();
            context.ChangeState(next_state);
            continue;
        } else if (status != AppState::SequenceStatus::kSuccess) {
            if (status == AppState::SequenceStatus::kErrorFatal) {
                // Senscord API returns fatal error
                ERR_PRINTF("[SEQ] SetDnn error fatal");
                fatal_error_ = true;
                break;
            }
        }
    }

    if (fatal_error_) {
        VER_PRINTF("[SEQ] MainLoop end fatal error");
        return -1;
    }

    VER_PRINTF("[SEQ] MainLoop end");
    return 0;
}

int32_t AppSequence::Finalize() {
    VER_PRINTF("[SEQ] Finalize start");
    if (fatal_error_) {
        int32_t counter = 0;
        while (!evp_thread_->IsThreadTerminated()) {
            // If fatal error occurred, output log periodically to push vbuf log buffer.
            if (counter >= FATAL_ERROR_LOG_COUNTER) {
                INFO_PRINTF("FATAL ERROR");
                counter = 0;
            } else {
                counter++;
            }
            usleep(FATAL_ERROR_LOG_USLEEP_INTERVAL);
        }
    }
    VER_PRINTF("[SEQ] Finalize end");
    return 0;
}

int AppSequence::ValidateParam(const void* param) {
    int ret = 0;
    MUTEX_LOCK();
    uint16_t num_of_analyzer = analyzers_.size();
    MUTEX_UNLOCK();
    for (uint16_t i = 0; i < num_of_analyzer; i++) {
        MUTEX_LOCK();
        auto analyzer = analyzers_[i];
        MUTEX_UNLOCK();
        AnalyzerBase::ResultCode result_code = analyzer->ValidateParam(param);
        if (result_code != AnalyzerBase::ResultCode::kOk) {
            ret += 1;
        }
    }
    return ret;
}

void AppSequence::ClearValidatingParam() {
    MUTEX_LOCK();
    uint16_t num_of_analyzer = analyzers_.size();
    MUTEX_UNLOCK();
    for (uint16_t i = 0; i < num_of_analyzer; i++) {
        MUTEX_LOCK();
        auto analyzer = analyzers_[i];
        MUTEX_UNLOCK();
        analyzer->ClearValidatingParam();
    }
}

void AppSequence::SetValidatedParam(const void* param) {
    MUTEX_LOCK();
    uint16_t num_of_analyzer = analyzers_.size();
    MUTEX_UNLOCK();
    for (uint16_t i = 0; i < num_of_analyzer; i++) {
        MUTEX_LOCK();
        auto analyzer = analyzers_[i];
        MUTEX_UNLOCK();
        analyzer->SetValidatedParam(param);
    }
}

void AppSequence::ReleaseFrameForContinueOrBreak(AppContext& context, AppState::SequenceStatus status) {
    switch (status) {
    case AppState::SequenceStatus::kNop:
    case AppState::SequenceStatus::kSuccess:
    case AppState::SequenceStatus::kSuccessCompleted:
    case AppState::SequenceStatus::kRetrySetProperty:
        break;
    case AppState::SequenceStatus::kContinue:
    case AppState::SequenceStatus::kError:
    case AppState::SequenceStatus::kErrorFatal:
    case AppState::SequenceStatus::kErrorChangeToOd:
        context.ReleaseFrame();
        break;
    default:
        break;
    }
}
