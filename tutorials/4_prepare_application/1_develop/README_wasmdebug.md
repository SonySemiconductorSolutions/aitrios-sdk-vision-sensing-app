# Running/Debugging Wasm in "**Vision and Sensing Application SDK**"

You can run and debug Wasm in the VS Code UI using [LLDB](https://lldb.llvm.org/) and [WAMR-IDE](https://github.com/bytecodealliance/wasm-micro-runtime/tree/main/test-tools/wamr-ide).<br>
The following APIs are mocked in the "**Vision and Sensing Application**" debugging :
- "**EVP SDK API**"
- "**SensCord SDK API**"
- "**Data Pipeline API**"

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

The data structure and value depend on the implementation of "**Vision and Sensing Application**".

Please edit the files to test if needed.

- Image Classification

    - **`./tutorials/4_prepare_application/1_develop/testapp/classification/output_tensor.jsonc`**

    - **`./tutorials/4_prepare_application/1_develop/testapp/classification/ppl_parameter.json`**

        - "dnn_output_classes" value must be the same as the number of classes of AI model.

- Object Detection

    - **`./tutorials/4_prepare_application/1_develop/testapp/objectdetection/output_tensor.jsonc`**

    - **`./tutorials/4_prepare_application/1_develop/testapp/objectdetection/ppl_parameter.json`**

> **NOTE**
> 
> **`output_tensor.jsonc`** and **`ppl_parameter.json`** are only used for running/debugging Wasm in "**Vision and Sensing Application SDK**".

> **NOTE**
> 
> For running the Wasm in the device, please specify the same content as the **`ppl_parameter.json`** to the **`PPLParameter`** of the **`StartUploadInferenceData`** which is a command parameter of "**Console for AITRIOS**".


## 3. Build and run test application with Wasm file for debugging

Test application located in [testapp](./testapp) loads Wasm file, Output Tensor jsonc file and ppl parameter json file.

And the test application calls "**Vision and Sensing Application**" interface (**`main`**).

To build and run test application, execute following command in TERMINAL.

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
> The script is for debugging the sample. Modify the Wasm file path in [start.sh](./start.sh) to match the location of the "**Vision and Sensing Application**" you created.  
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

## 4. Debugging Wasm

1. Press F5 key to attach the debugger. Then "Could not load source" 
 message will be displayed in the top pane, so remove the message tab.

2. Set some breakpoints in following sample code.

- Classification
  - [vision_app_classification.cpp](./sdk/sample/vision_app/single_dnn/classification/src/vision_app_classification.cpp)
  - [analyzer_classification.cpp](./sdk/sample/post_process/classification/src/analyzer_classification.cpp)

- Object Detection
  - [vision_app_objectdetection.cpp](./sdk/sample/vision_app/single_dnn/objectdetection/src/vision_app_objectdetection.cpp)
  - [analyzer_objectdetection.cpp](./sdk/sample/post_process/objectdetection/src/analyzer_objectdetection.cpp)

3. Press the F5 key, then the program continues and stops at the breakpoint.

4. Review variables in VARIABLES. And review log output in TERMINAL.  

    see [Debugging in Visual Studio Code](https://code.visualstudio.com/Docs/editor/debugging) for details.  

> **NOTE**
> 
> The mock implements following behavior.
>
> - Execute Wasm's **`ConfigurationCallback`** only once
> - Execute Wasm's analyze method (for example, **`PPL_ObjectDetectionSsdAnalyze`**) only once
> - After Wasm's **`SendDataDoneCallback`** called, Wasm application exits
> - To achieve preceding behavior, **`senscord_stream_get_frame`** sometimes returns less than 0 and **`senscord_get_last_error`**'s status.cause is sometimes **`SENSCORD_ERROR_TIMEOUT`**

## 5. Stop the test application

Press [CTRL + C] keys in TERMINAL.
