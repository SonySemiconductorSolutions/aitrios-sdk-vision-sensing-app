#!/bin/bash
# Copyright 2022-2023 Sony Semiconductor Solutions Corp. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

NAME_IMAGE='app_build_env:1.0.0';

BASE_DIRECTORY=${PWD}

cd "$(dirname "$0")"

usage_exit() {
    echo "usage	: ./start.sh <Options>"
    echo ""
    echo " <Options>"
    echo "    -t : (mandatory) specify build/run Wasm type. value is [ic|od|switchdnn]."
    echo "    -d : (optional) build for debugging. start and wait for attaching debugger."
    echo "    -o : (optional) specify output tensor jsonc file for debugging."
    echo "    -p : (optional) specify ppl parameter json file for debugging."
    echo "    -f : (Optional) Specify the user-created Wasm file you want to debug. value is the absolute path to the file. The file must be in ./tutorials/4_prepare_application/1_develop/**/*."
    echo "    -m : (Optional) build with Memory consumption display API."
    echo ""
    echo " Example : ./tutorials/4_prepare_application/1_develop/start.sh -d -t od -o ./tutorials/4_prepare_application/1_develop/testapp/objectdetection/output_tensor.jsonc"
    echo "           means that Wasm type is od, specified output tensor file and built/ran for debugging."
    exit 1
}

LOAD_PGM=none
DEBUGGER=""
PPL_PARAMETER_FILE=
OUTPUT_TENSOR_FILE=
TESTAPP_CONFIG_FILE="${PWD}/testapp/configuration/testapp_configuration.json"
USER_WASM_FILE=
DUMP_MEMORY_CONSUMPTION=""

while getopts dmo:p:f:t: option
do
  case $option in
    d)
      DEBUGGER="-d";;
    o)
      OUTPUT_TENSOR_FILE=${OPTARG};;
    p)
      PPL_PARAMETER_FILE=${OPTARG};;
    f)
      USER_WASM_FILE=${OPTARG};;
    t)
      LOAD_PGM=${OPTARG};;
    m)
      DUMP_MEMORY_CONSUMPTION="-m";;
    ?)
      usage_exit ;;
    :)
      usage_exit ;;
  esac
done

if [ $LOAD_PGM != "ic" ] && [ $LOAD_PGM != "od" ] && [ $LOAD_PGM != "switchdnn" ]; then
    usage_exit
fi

# Switching Wasm References
if [ "$USER_WASM_FILE" = "" ]; then
    if [ $LOAD_PGM = "ic" ]; then
        if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
            WASM_FILE=${PWD}/sdk/sample/build/debug/vision_app_classification.wasm
        else
            WASM_FILE=${PWD}/sdk/sample/build/release/vision_app_classification.wasm
        fi
    elif [ $LOAD_PGM = "od" ]; then
        if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
            WASM_FILE=${PWD}/sdk/sample/build/debug/vision_app_objectdetection.wasm
        else
            WASM_FILE=${PWD}/sdk/sample/build/release/vision_app_objectdetection.wasm
        fi
    else
        if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
            WASM_FILE=${PWD}/sdk/sample/build/debug/vision_app_switch_dnn.wasm
        else
            WASM_FILE=${PWD}/sdk/sample/build/release/vision_app_switch_dnn.wasm
        fi
    fi
else
    if [ ! -e $USER_WASM_FILE ]; then
        echo "Wasm file not found";
        exit 0
    fi

    WASM_FILE=$USER_WASM_FILE
fi

# Determining the default input file
if [ "$PPL_PARAMETER_FILE" = "" ]; then
    if [ $LOAD_PGM = "ic" ]; then
        PPL_PARAMETER_FILE="${PWD}/testapp/classification/ppl_parameter.json"
    elif [ $LOAD_PGM = "od" ]; then
        PPL_PARAMETER_FILE="${PWD}/testapp/objectdetection/ppl_parameter.json"
    else
        PPL_PARAMETER_FILE="${PWD}/testapp/switch_dnn/ppl_parameter.json"
    fi
fi

if [ "$OUTPUT_TENSOR_FILE" = "" ]; then
    if [ $LOAD_PGM = "ic" ]; then
        OUTPUT_TENSOR_FILE="${PWD}/testapp/classification/output_tensor.jsonc"
    elif [ $LOAD_PGM = "od" ]; then
        OUTPUT_TENSOR_FILE="${PWD}/testapp/objectdetection/output_tensor.jsonc"
    else
        OUTPUT_TENSOR_FILE="${PWD}/testapp/switch_dnn/output_tensor.jsonc"
    fi 
fi

# Validating testapp_configuration.json
python ${PWD}/testapp/configuration/json_validation.py
if [ $? != 0 ]; then
    echo "testapp_configuration.json validation error";
    exit 0
fi

LOADER_DIR="${PWD}/testapp/build/loader"
DST_PPL_PARAMETER_FILE="$LOADER_DIR/ppl_parameter.json"
DST_OUTPUT_TENSOR_FILE="$LOADER_DIR/output_tensor.jsonc"
DST_TESTAPP_CONFIG_FILE="$LOADER_DIR/testapp_configuration.json"

NATIVE_LIBS_ARGS=" -n libdev_mock.so"

BUILD_STATE_FILE=${PWD}/sdk/sample/build_state.dat

# Build Wasm
if [ "$USER_WASM_FILE" = "" ]; then
    ./build.sh -t $LOAD_PGM $DEBUGGER $DUMP_MEMORY_CONSUMPTION

    # Exit if Wasm build fails
    if [ ! -e "$BUILD_STATE_FILE" ]; then
        exit 1;
    fi
fi

if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
    docker build . -f $PWD/Dockerfile -t $NAME_IMAGE --network host
fi

INTERACTIVE_OPTION="-i"

if [ -n "$DEBIAN_FRONTEND" ] && [ $DEBIAN_FRONTEND = "noninteractive" ]; then
    INTERACTIVE_OPTION=""
fi

MOUNT_DIRECTORY=${PWD}

PARM_ALL="-f ${WASM_FILE}${NATIVE_LIBS_ARGS} $@"

# Build test application
docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./build.sh"

# Copy files
if [ "${PPL_PARAMETER_FILE:0:1}" != "/" ]; then
    cp -f $BASE_DIRECTORY/$PPL_PARAMETER_FILE $DST_PPL_PARAMETER_FILE
    if [ $? -gt 0 ]; then
        exit 1
    fi
else
    cp -f $PPL_PARAMETER_FILE $DST_PPL_PARAMETER_FILE
    if [ $? -gt 0 ]; then
        exit 1
    fi
fi

if [ "${OUTPUT_TENSOR_FILE:0:1}" != "/" ]; then
    cp -f $BASE_DIRECTORY/$OUTPUT_TENSOR_FILE $DST_OUTPUT_TENSOR_FILE
    if [ $? -gt 0 ]; then
        rm -rf "$DST_PPL_PARAMETER_FILE"
        exit 1
    fi
else
    cp -f $OUTPUT_TENSOR_FILE $DST_OUTPUT_TENSOR_FILE
    if [ $? -gt 0 ]; then
        rm -rf "$DST_PPL_PARAMETER_FILE"
        exit 1
    fi
fi

cp -f $TESTAPP_CONFIG_FILE $DST_TESTAPP_CONFIG_FILE
if [ $? -gt 0 ]; then
    rm -rf "$DST_PPL_PARAMETER_FILE"
    rm -rf "$DST_OUTPUT_TENSOR_FILE"
    exit 1
fi

# Run test application with Wasm
docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/testapp/ && ./run.sh $PARM_ALL"

rm -rf "$DST_PPL_PARAMETER_FILE"
rm -rf "$DST_OUTPUT_TENSOR_FILE"
rm -rf "$DST_TESTAPP_CONFIG_FILE"
