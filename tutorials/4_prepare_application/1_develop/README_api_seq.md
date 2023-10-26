# Vision and Sensing Application API Sequence

The following is a sequence diagram that illustrates the basic processing of "**Vision and Sensing Application**". This sequence is based on the processing of the sample code.
See the [sample code](./sdk/sample/vision_app/single_dnn/) and the [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/documents/specifications) for more information on the actual processing and how to use these APIs.

## Sequence

<!-- source,mermaid alt text: Sequence -->
```source,mermaid
sequenceDiagram
participant Console as Console
participant Native as Native (Firmware)
box Wasm Native Library (Firmware)
participant evp as EVP SDK API
participant senscord as SensCord SDK API
participant dpl as Data Pipeline API
end
participant Wasm as Wasm<br> (Vision and Sensing Application)
participant Wasm_Thread as Wasm thread<br> (Vision and Sensing Application)


%% Initialize
activate Console
activate Native

Native->>Wasm: Main()
activate Wasm

Wasm->>dpl: SessInit()
activate dpl
dpl-->>Wasm: 
deactivate dpl

Wasm->>dpl: SessRegisterSendDataCallback()
activate dpl
dpl-->>Wasm: 
deactivate dpl

Wasm-)Wasm_Thread: pthread_create
activate Wasm_Thread

Wasm_Thread->>evp: EVP_initialize()
activate evp
evp-->>Wasm_Thread: 
deactivate evp

Wasm_Thread->>evp: EVP_setConfigurationCallback()
activate evp
evp-->>Wasm_Thread: 
deactivate evp

%% Set Configuration
loop Loop in Wasm thread
    Wasm_Thread->>evp: EVP_processEvent()
    activate evp
    evp-->>Wasm_Thread: 
    deactivate evp

    %% StartUploadInferenceResult
    opt Call StartUploadInferenceResult from Console
    Console-)Native: StartUploadInferenceResult
    activate Native
    Native->>Native: Receive PPL Parameter
    Note right of Native: PPL Parameter is passed once<br> in a StartUploadInferenceResult call.
    Native->>evp: Set Configuration
    activate evp
    Note over evp: PPL Parameter is set in Firmware.

    opt PPL Parameter is set
    Note right of evp: Once PPL Parameter is set,<br> ConfigurationCallback is called only once.
    evp->>Wasm_Thread: ConfigurationCallback
    activate Wasm_Thread
    Note over Wasm_Thread: PPL Parameter is set in Wasm.
    Wasm_Thread-->> evp: 
    deactivate Wasm_Thread
    end

    evp-->>Native: 
    deactivate evp
    end
end

loop Loop in Main
    %% Get Output Tensor
    Wasm->>senscord: senscord_stream_get_frame
    activate senscord

    alt Not receiving frame data from Firmware yet
        Note over senscord: Wait frame data.
    else Receive frame data from Firmware
        Note right of Native: By calling StartUploadInferenceResult,<br> Firmware starts retrieving frame data.
        Native->>Native: Start shooting with IMX500
        Native->>Native: Get frame data
        Native->>senscord: Push frame data
        activate senscord
        Note over senscord: Receive frame data.
        senscord-->>Native: 
        deactivate Native
        senscord-->>Wasm: 
        deactivate senscord
    deactivate senscord
    end

    %% Analyze
    Wasm->>Wasm: Get Output Tensor from frame data
    Wasm->>Wasm: Analyze and Serialize
    Note right of Wasm: PPL Parameter set in Wasm is used<br> for analysis processing.

    %% Send data to cloud
    Wasm->>dpl: SessMalloc()
    activate dpl
    dpl-->>Wasm: 
    deactivate dpl

    Wasm->>dpl: SessSendData()
    activate dpl
    dpl->>Native: 
    activate Native
    Native->>Console: Send analyzed data to Console
    Native-->>dpl: 
    deactivate Native
    dpl-->>Wasm: 
    deactivate dpl

    dpl->>Wasm: SendDataDoneCallback
    activate Wasm

    Wasm->>dpl: SessFree()
    activate dpl
    dpl-->>Wasm: 
    deactivate dpl

    Wasm-->>dpl: 
    deactivate Wasm
end

%% Finalize
Wasm->>Wasm_Thread: pthread_join
Wasm_Thread-->>Wasm: 
deactivate Wasm_Thread

Wasm->>dpl: SessExit()
activate dpl
dpl-->>Wasm: 
deactivate dpl

Wasm-->>Native: 
deactivate Wasm

deactivate Console
deactivate Native
```

> **NOTE**
>
>At **`StartUploadInferenceResult`**, "**Console**" passes the Configuration (PPL Parameter) to a device.
