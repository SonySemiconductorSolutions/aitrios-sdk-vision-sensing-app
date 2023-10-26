# "**Vision and Sensing Application**" Migration Guide from SDK v0.2 to v1.0

If you have already developed the "**Vision and Sensing Application**" using SDK v0.2,
you need to modify the source code of the "**Vision and Sensing Application**" to migrate to SDK v1.0.

## Overview

In SDK v0.2, the "**Vision and Sensing Application**" can do:

- Customize behavior by changing parameters using **`PPLParameter`** in Command Parameter File of StartUploadInferenceData
- Receive and Analyze IMX500 Output Tensor data
- Upload the analyzed Metadata of inference result

Additionally, in SDK v1.0, the "**Vision and Sensing Application**" also can do:

- Customize a whole behavior like sequence and timing of getting Output Tensor, analyzing and uploading by custom implementation in event loop.

## Sequence of SDK v0.2

Native (Firmware) has a main loop. Wasm ("**Vision and Sensing Application**") functions are called from Native.

<!-- source,mermaid alt text: Sequence of SDK v0.2 -->
```source,mermaid
sequenceDiagram
participant Native as Native (Firmware)
participant Wasm as Wasm (Vision and Sensing Application)
Native->>Native: Receive PPL Parameter from Console
Native->>Wasm: PPL_Initialize()
Wasm->>Wasm: Get PPL Parameter
Wasm-->>Native: 
alt Loop
    Native->>Wasm: PPL_Analyze()
    Wasm->>Wasm: Analyze and Serialize
    Wasm-->>Native: 
    Native->>Native: Send to Console
    Native->>Wasm: PPL_ResultRelease()
    Wasm-->>Native: 
end
Native->>Wasm: PPL_Finalize()
Wasm-->>Native: 
```

## Sequence of SDK v1.0

Wasm has a main loop and a thread. Wasm functions are called from Wasm and Native callback.

> **NOTE**
>
> This sequence is based on the processing of the sample code.

<!-- source,mermaid alt text: Sequence of SDK v1.0 -->
```source,mermaid
sequenceDiagram
participant Native as Native (Firmware)
participant Wasm_Lib as Wasm Native Library (Firmware)
participant Wasm as Wasm (Vision and Sensing Application)
participant Wasm_Thread as Wasm thread (Vision and Sensing Application)
Native->>Wasm: Main()
Wasm->>Wasm_Lib: SessInit()
Wasm_Lib-->>Wasm: 
Wasm->>Wasm_Lib: SessRegisterSendDataCallback()
Wasm_Lib-->>Wasm: 
Wasm->>Wasm_Thread: pthread_create
Wasm_Thread-->>Wasm: 
Wasm_Thread->>Wasm_Lib: EVP_setConfigurationCallback()
Wasm_Lib-->>Wasm_Thread: 
loop Loop in Wasm thread
    Wasm_Thread->>Wasm_Lib: EVP_processEvent()
    Wasm_Lib-->>Wasm_Thread: 
    alt Receive PPL Parameter
    Native->>Native: Receive PPL Parameter from Console
    Native->>Wasm_Lib: Set Configuration
    Wasm_Lib->>Wasm_Thread: ConfigurationCallback
    Wasm_Thread->>Wasm_Thread: Get PPL Parameter
    Wasm_Thread-->> Wasm_Lib: 
    Wasm_Lib-->>Native: 
    end
end
loop Loop in Main
    Wasm->>Wasm_Lib: senscord_channel_get_raw_data()
    Wasm_Lib-->>Wasm: 
    Wasm->>Wasm: Analyze and Serialize
    Wasm->>Wasm_Lib: SessMalloc()
    Wasm_Lib-->>Wasm: 
    Wasm->>Wasm_Lib: SessSendData()
    Wasm_Lib->>Native: 
    Native->>Native: Send to Console
    Native-->>Wasm_Lib: 
    Wasm_Lib-->>Wasm: 
    Wasm_Lib->>Wasm: SendDataDoneCallback
    Wasm->>Wasm_Lib: SessFree()
    Wasm_Lib-->>Wasm: 
    Wasm-->>Wasm_Lib: 
end
Wasm->>Wasm_Thread: pthread_join
Wasm_Thread-->>Wasm: 
Wasm->>Wasm_Lib: SessExit()
Wasm_Lib-->>Wasm: 
Wasm-->>Native: 
```

## Migration steps

The sample code of "**Vision and Sensing Application**" in SDK v1.0 is implemented as the same behavior and sequence of SDK v0.2.

So please use the [sample code](../../../tutorials/4_prepare_application/1_develop/sdk/sample/) of "**Vision and Sensing Application**" in SDK v1.0 as base source code.

And please move your custom code of "**Vision and Sensing Application**" (modified, added and removed code based on the sample code of SDK v0.2) from SDK v0.2 to SDK v1.0.

1. Place your FlatBuffers .fbs file to [schema folder](../../../tutorials/4_prepare_application/1_develop/sdk/schema/)

2. Regenerate FlatBuffers header file. Please see [Generate a C++ header file from the FlatBuffers schema file](../../../tutorials/4_prepare_application/1_develop/README.md#2-generate-a-c-header-file-from-the-flatbuffers-schema-file)

3. Move the generated header file to [sample folder (for example, objectdetection)](../../../tutorials/4_prepare_application/1_develop/sdk/sample/include/objectdetection/)

4. Migrate configuration logic of PPL Parameter

    Please move your custom logic from SDK v0.2 to SDK v1.0.

    |   | SDK v0.2 | SDK v1.0 |
    | ------------- | ------------- | ------------- |
    | Receive PPL Parameter logic | [PPL_ObjectDetectionSsdParamInit() in json_parse()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L352)  | [PPL_ObjectDetectionSsdParamInit() in json_parse()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp#L202) |

5. Migrate analyzing logic of Output Tensor

    Please move your custom logic from SDK v0.2 to SDK v1.0.

    |   | SDK v0.2 | SDK v1.0 |
    | ------------- | ------------- | ------------- |
    | Analyze Output Tensor logic  | [analyseObjectDetectionSsdOutput() in PPL_ObjectDetectionSsdAnalyze()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L385)  | [analyseObjectDetectionSsdOutput() in PPL_ObjectDetectionSsdAnalyze()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp#L114) |

6. Migrate serialization logic of metadata of inference result

    Please move your custom logic from SDK v0.2 to SDK v1.0.

    |   | SDK v0.2 | SDK v1.0 |
    | ------------- | ------------- | ------------- |
    | Serialization logic  | [createSSDOutputFlatbuffer() in PPL_ObjectDetectionSsdAnalyze()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L389)  | [createSSDOutputFlatbuffer() in PPL_ObjectDetectionSsdAnalyze()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp#L118) |

7. Build "**Vision and Sensing Application**". See [Build the "**Vision and Sensing Application**"](../../../tutorials/4_prepare_application/1_develop/README.md#build-the-vision-and-sensing-application)

8. (Optional) Running/debugging Wasm in "**Vision and Sensing Application SDK**". See [Running/debugging Wasm in "**Vision and Sensing Application SDK**"](../../../tutorials/4_prepare_application/1_develop/README_wasmdebug.md).

## Appendix

### Difference of Wasm public interface

|   | SDK v0.2 | SDK v1.0 |
| ------------- | ------------- | ------------- |
| Wasm public interface  | PPL_Initialize()<br>PPL_Analyze()<br>PPL_ResultRelease()<br>PPL_Finalize()<br>PPL_GetPplVersion() | main() |

### Difference of function

|   | SDK v0.2 | SDK v1.0 |
| ------------- | ------------- | ------------- |
| Receive PPL Parameter interface  | [PPL_Initialize()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L115)  | a part of [ConfigurationCallback()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/vision_app/single_dnn/objectdetection/src/vision_app_objectdetection.cpp#L349) |
| Receive PPL Parameter logic | [json_parse()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L307)  | [json_parse()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp#L156) |
| Analyze and serialize Output Tensor interface  | [PPL_Analyze()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L161)  | a part of [main()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/vision_app/single_dnn/objectdetection/src/vision_app_objectdetection.cpp#L180) |
| Analyze and serialize Output Tensor logic  | [PPL_ObjectDetectionSsdAnalyze()](https://github.com/SonySemiconductorSolutions/aitrios-sdk-vision-sensing-app/blob/v0.2.0/tutorials/4_prepare_application/1_develop/sample/objectdetection/ppl_objectdetection.cpp#L358)  | [PPL_ObjectDetectionSsdAnalyze()](../../../tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp#L87) |
