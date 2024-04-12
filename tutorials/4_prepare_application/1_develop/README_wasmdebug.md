# Running/Debugging Wasm in "**Edge Application SDK**"

You can run and debug Wasm in the VS Code UI using [LLDB](https://lldb.llvm.org/), [WAMR-IDE](https://github.com/bytecodealliance/wasm-micro-runtime/tree/main/test-tools/wamr-ide) and test application.<br>

The following APIs are mocked by test application in the "**Edge Application**" debugging :
- "**EVP API**"
- "**SensCord API**"
- "**Data Pipeline API**"

The test application can input following parameters and these are used to mock APIs. <br>
- Output Tensor
- PPL Parameter
- test application initial configuration



## 1. Initialize (only the first time)

1. Get LLDB and WAMR-IDE binary file

    Run following command in TERMINAL.
    ```bash
    .devcontainer/wamr-ide/download.sh
    ```

2. Install WAMR-IDE

    In VS Code UI, install wamride-1.1.2.vsix extension.

    Select "Extensions" from the Activity bar, - "Views and More Actions..." - "Install from VSIX" - Select /home/vscode/wamride-1.1.2.vsix.

    If "NO LLDB instance found. Setup now?" dialog displayed, press "skip" button. (Even if you don't press "skip" but "setup" button, you still need to "install LLDB to WAMR-IDE" in the next step.)

    If "Completed installing WAMR-IDE extension from VSIX." dialog displayed, press "Reload Now" button.

    Select "Run and Debug" from the Activity bar - "Show all automatic debug configurations" - "Add configuration..." - "WAMR lldb debugger".

    (If "WAMR lldb debugger" is not displayed, select "Create a launch.json file" - select "WAMR lldb debugger".)

    Then .vscode/launch.json is generated like following:
    ```json
    {
        // Use IntelliSense to learn about possible attributes.
        // Hold the pointer over to view descriptions of existing attributes.
        // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
        "version": "0.2.0",
        "configurations": [
            {
                "type": "wamr-debug",
                "request": "attach",
                "name": "Debug",
                "stopOnEntry": true,
                "attachCommands": [
                    "process connect -p wasm connect://127.0.0.1:1234"
                ]
            }
        ]
    }
    ```

3. Install LLDB to WAMR-IDE

    Run following command in TERMINAL.

    - In Codespaces environment
    ```bash
    .devcontainer/wamr-ide/install_codespaces.sh
    ```

    - In Dev Container on Local PC environment
    ```bash
    .devcontainer/wamr-ide/install_localdevcontainer.sh
    ```

## 2. Prepare input parameters

The Output Tensor and PPL Parameter are stored in the following location.

The data structure and value depend on the implementation of "**Edge Application**".

Please edit the files to test if needed.

- Image Classification

    - **`./tutorials/4_prepare_application/1_develop/testapp/classification/output_tensor.jsonc`**

    - **`./tutorials/4_prepare_application/1_develop/testapp/classification/ppl_parameter.json`**

        - "dnn_output_classes" value must be the same as the number of classes of AI model.

- Object Detection

    - **`./tutorials/4_prepare_application/1_develop/testapp/objectdetection/output_tensor.jsonc`**

    - **`./tutorials/4_prepare_application/1_develop/testapp/objectdetection/ppl_parameter.json`**

- Switch DNN

    - **`./tutorials/4_prepare_application/1_develop/testapp/switch_dnn/output_tensor.jsonc`**

    - **`./tutorials/4_prepare_application/1_develop/testapp/switch_dnn/ppl_parameter.json`**

You can change test application initial configuration.
Please edit the following file to test if needed.
- **`./tutorials/4_prepare_application/1_develop/testapp/configuration/testapp_configuration.json`**. <br>

The testapp_configuration.json has following parameters:
    
  - `input_tensor_dewarp_crop_property`
  - `senscord_dnn_property`
  
These parameters are set as the initial return values of mocked **`senscord_stream_get_property`** and **`senscord_channel_get_property`**. See ["**SensCord API Specification**"](https://developer.aitrios.sony-semicon.com/en/file/download/aitrios-apispec-senscordsdk-v1-0-1-en) for more information on the APIs.
For details on the parameter settings, see [testapp_configuration_schema.json](./testapp/configuration/testapp_configuration_schema.json).

> **NOTE**
> 
> **`output_tensor.jsonc`**, **`ppl_parameter.json`** and **`testapp_configuration.json`** are only used for running/debugging Wasm in "**Edge Application SDK**".

> **NOTE**
> 
> For running the Wasm in the device, please specify the same content as the **`ppl_parameter.json`** to the **`PPLParameter`** of the **`StartUploadInferenceData`** which is a command parameter of "**Console for AITRIOS**".

## 3. Check memory consumption

You can check Wasm memory consumption after the Wasm exit by running test application. The Wasm memory consumption is displayed as console log output.

Example of console log output when running test application:

```
･･･

Memory consumption summary (bytes):
WASM module memory consumption, total size: 16223
    module struct size: 304
    types size: 532
    imports size: 1332
    funcs size: 13579  <-------------------------------------------------- *1
    tables size: 168
    memories size: 16
    globals size: 96
    exports size: 72
    table segs size: 44
    data segs size: 80
    const strings size: 0
    aot code size: 0
WASM module inst memory consumption, total size: 72972
    module inst struct size: 668
    memories size: 65740     <-------------------------------------------- *2
        app heap size: 0
    tables size: 4
    functions size: 6440
    globals size: 96
    exports size: 24
Exec env memory consumption, total size: 34472
    exec env struct size: 1704
        block addr cache size: 1536
    stack size: 32768  <-------------------------------------------------- *3

Total memory consumption of module, module inst and exec env: 123667  <--- *4
Total interpreter stack used: 2792  <------------------------------------- *5
Total auxiliary stack used: 2480  <--------------------------------------- *6
Total app heap used: 0  <------------------------------------------------- *7
native_lib_count: 1
```

Explanation of above output contents:

> **NOTE**
> 
> This feature is using the experimental feature of WebAssembly Micro Runtime (WAMR). So the following descriptions are inferred from our verification. The accurate information can be found [here](https://github.com/bytecodealliance/wasm-micro-runtime/blob/WAMR-1.1.2/doc/build_wamr.md#enable-performance-profiling-experiment).

- Wasm memory consumption

    | Marking | Summary | Description | Items to check | Remarks |
    |:--:|:---|:---|:---|:---|
    |*1|WASM module memory consumption|Memory used to load Wasm|funcs size|Memory consumption of executable code|
    |*2|WASM module inst memory consumption|Memory used when instantiating Wasm module|memories size|Memory consumption for dynamic memory allocation|
    |*3|Exec env memory consumption|Memory to run Wasm. Test application uses|stack size|Maximum stack value to use for Test application when executing Wasm|

- Total memory, stack and heap

    | Marking | Summary | Description |
    |:--:|:---|:---|
    |*4|Total memory consumption of module, module inst and exec env|Total memory consumption types|
    |*5|Total interpreter stack used|Test application stack consumption|
    |*6|Total auxiliary stack used|Wasm stack consumption|
    |*7|Total app heap used|Heap consumption|

If you want to check Wasm memory consumption at any line of the Wasm source code, please do following steps to call **`Memory consumption display API`** in the Wasm source code.

> **NOTE**
> 
> **`Memory consumption display API`** is an native API implemented only in this test application. The API is not available in edge AI device. So if you deploy Wasm including calling the API to edge AI device, runtime error occurs. Before deploying to edge AI device, please remove the header file definition for the API and delete calling the API in the Wasm source code.

1. Include header file.

    ```c
    #include "vision_app_memory_dump.h"  // Add this line.
    ```

2. Add calling **`Memory consumption display API`**.

    API definition:
    ```c
    void TESTAPP_dump_mem_consumption();
    ```

    | parameters | returns |
    |:---:|:---:|
    | none | none |

    Usage example:
    ```c
        ret = senscord_channel_get_raw_data(channel, &raw_data);
        if (ret < 0) {
            ERR_PRINTF("senscord_channel_get_raw_data : ret=%d", ret);
            senscord_get_last_error(&status);
            PrintError(status);
                break;
        }
        TESTAPP_dump_mem_consumption();  // Add this line.
    ```

3. When running test application, please append the "-m" option to [start.sh](./start.sh).

> **NOTE**
> 
> If you call the **`Memory consumption display API`** in the Wasm source code and execute start.sh without specifying the "-m" option, you will get an error and will not be able to execute.

## 4. Build and run test application with Wasm file for debugging

Test application located in [testapp](./testapp) loads Wasm file, Output Tensor jsonc file, ppl parameter json file and test application configuration file.

And the test application calls "**Edge Application**" interface (**`main`**).

To build and run test application, execute following command in TERMINAL.

- **`start.sh`** command parameters used in this chapter:  

    | Parameter | mandatory<br> / optional<br> | Description |
    |:--:|:--:|:---|
    | -t | mandatory | specify build/run Wasm type. value is [ic/od/switchdnn]. |
    | -d | optional | build for debugging. start and wait for attaching debugger. |
    | -o | optional | specify output tensor jsonc file for debugging. |
    | -p | optional | specify ppl parameter json file for debugging. |
    | -f | optional | Specify the user-created Wasm file you want to debug. value is the absolute path to the file. The file must be in ./tutorials/4_prepare_application/1_develop/**/*. |
    | -m | optional | build with Memory consumption display API. If not specified, you will get an error and will not be able to execute. |

- Image Classification (default setting)

    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t ic
    ```

    Or (specify input file setting)
    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t ic -o <file1> -p <file2>
    ```

- Object Detection (default setting)
    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t od
    ```

    Or (specify input file setting)
    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t od -o <file1> -p <file2>  
    ```

- Switch DNN (default setting)
    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t switchdnn
    ```

    Or (specify input file setting)  
    ```bash
    $ ./tutorials/4_prepare_application/1_develop/start.sh -d -t switchdnn -o <file1> -p <file2>  
    ```

> **NOTE**
> 
> **`<file1>`** Path of the Output Tensor file.　  
> **`<file2>`** Path of the PPL Parameter file.   

Then the following message is displayed in TERMINAL.

```sh
Debug server listening on 127.0.0.1:1234  
Debug port : 1234
```

> **NOTE**
> 
> If you want to run test app and Wasm without debugging, run preceding command without "-d" option.

> **NOTE**
> 
> For displaying description of command options, run following command.
> ```sh
> ./tutorials/4_prepare_application/1_develop/start.sh -h
> ```

> **NOTE**
> 
> The script is for debugging the sample. Modify the Wasm file path in [start.sh](./start.sh) to match the location of the "**Edge Application**" you created.  
> For example:
> ```sh
> WASM_FILE=${PWD}/sdk/sample/build/debug/vision_app_objectdetection.wasm
> ```
> And modify build command in [build.sh](./build.sh). Detail is described in [README.md](./README.md#2-build).  
> Source files and Wasm file must be in **`./tutorials/4_prepare_application/1_develop/**/*`** .

> **NOTE**
>
> Internal information of [start.sh](./start.sh).  
> The script has following sections.
> 
> - **`build.sh`** command parameters used in this chapter:
> 
>   | Parameter | mandatory<br> / optional<br> | Description |
>   |:--:|:--:|:---|  
>   | -t | optional | specify build Wasm type. value is [ic/od/switchdnn]. |
>   | -d | optional | build for debugging. |
>   | -c | optional | clean all Wasm object. |
>   | -C | optional | clean all Wasm object and Docker image. |
>   | -s | optional | run AOT file size check (only release build available) |
>   | -m | optional | build with Memory consumption display API. If not specified, you will get an error. |
> 
> 1. Build Wasm 
>     ```sh
>     ./build.sh -t $LOAD_PGM $DEBUGGER 
>     ```
>     For example, Wasm type is od and debug build:
>     ```sh
>     ./build.sh -t od -d
>     ```
>     For example, Wasm type is ic and release build:
>     ```sh
>     ./build.sh -t ic
>     ```
> 
> 2. Build test application 
>     ```sh
>     docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./build.sh"
>     ```
> 
> 3. Copy files
>     ```sh
>     cp -f $PPL_PARAMETER_FILE $DST_PPL_PARAMETER_FILE
>     ```
>     ```sh
>     cp -f $OUTPUT_TENSOR_FILE $DST_OUTPUT_TENSOR_FILE
>     ```
>
>     Copy ppl_parameter.json and output_tensor.jsonc to the test application's folder ( **`./tutorials/4_prepare_application/1_develop/testapp/build/loader/`** ).
> 
> 4. Run test application with Wasm
>     ```sh
>     docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./run.sh $PARM_ALL"
>     ```
>     For example, Wasm type is od and running with debugging:
>     ```sh
>     docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./run.sh $NATIVE_LIBS_ARGS -t od -d -f <file3>"
>     ```
>     For example, Wasm type is ic and running without debugging:
>     ```sh
>     docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./run.sh $NATIVE_LIBS_ARGS -t ic -f <file3>"
>     ```
>     **`<file3>`** Absolute path of the Wasm file.  

## 5. Debugging Wasm

1. Press F5 key to attach the debugger. Then "Could not load source" 
 message will be displayed in the top pane, so remove the message tab.

2. Set some breakpoints in following sample code.

- Classification
  - [vision_app_classification.cpp](./sdk/sample/vision_app/single_dnn/classification/src/vision_app_classification.cpp)
  - [analyzer_classification.cpp](./sdk/sample/post_process/classification/src/analyzer_classification.cpp)

- Object Detection
  - [vision_app_objectdetection.cpp](./sdk/sample/vision_app/single_dnn/objectdetection/src/vision_app_objectdetection.cpp)
  - [analyzer_objectdetection.cpp](./sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp)

- Switch DNN
  - [vision_app_switch_dnn.cpp](./sdk/sample/vision_app/switch_dnn/switch_od_ic/src/vision_app_switch_dnn.cpp)
  - [analyzer_switch_dnn.cpp](./sdk/sample/post_process/switch_dnn/src/analyzer_switch_dnn.cpp)

3. Press the F5 key, then the program continues and stops at the breakpoint.

4. Review variables in VARIABLES. And review log output in TERMINAL.  

    see [Debugging in Visual Studio Code](https://code.visualstudio.com/Docs/editor/debugging) for details.  

> **NOTE**
> 
> The mock implements following behavior.
>
> - Execute Wasm's **`ConfigurationCallback`** only once
> - The Wasm application exits after all Wasm's **`SendDataDoneCallback`** is called.
> - To achieve preceding behavior, **`senscord_stream_get_frame`** sometimes returns less than 0 and **`senscord_get_last_error`**'s status.cause is sometimes **`SENSCORD_ERROR_TIMEOUT`**

## 6. Stop the test application

Press [CTRL + C] keys in TERMINAL.
