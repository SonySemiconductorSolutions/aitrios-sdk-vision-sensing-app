#!/bin/bash
# Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
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

ZONE_DETECTION_SAMPLE='.devcontainer/dependencies/aitrios-sdk-zone-detection-webapp-cs/sample'
VNS_ENV='tutorials/4_prepare_application/1_develop'
# set to SDK root directory
cd ../../../

usage_exit() {
    echo "usage	: ./start.sh <Options>"
    echo ""
    echo " <Options>"
    echo "    -d : (optional) build for debugging. start and wait for attaching debugger."
    echo "    -o : specify output tensor jsonc file for debugging."
    echo "    -p : specify ppl parameter json file for debugging."
    echo "    -f : (Optional) Specify the user-created Wasm file you want to debug. value is the absolute path to the file. The file must be in ./tutorials/4_prepare_application/1_develop/**/*."
    echo ""
    echo " Example : ./application/start.sh -d -o ./application/output_tensor.jsonc"
    echo "           means that specified output tensor file and built/ran for debugging."
    exit 1
}

LOAD_PGM=none
DEBUGGER=""
PPL_PARAMETER_FILE=
OUTPUT_TENSOR_FILE=
TESTAPP_CONFIG_FILE="${PWD}/tutorials/4_prepare_application/1_develop/testapp/configuration/testapp_configuration.json"
USER_WASM_FILE=

while getopts do:p:f:t: option
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
    ?)
      usage_exit ;;
    :)
      usage_exit ;;
  esac
done

# Switching Wasm References
if [ "$USER_WASM_FILE" = "" ]; then
    if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
        WASM_FILE=${PWD}/${ZONE_DETECTION_SAMPLE}/sample/vision_app/single_dnn/zonedetection/build/debug/vision_app_zonedetection.wasm
    else
        WASM_FILE=${PWD}/${ZONE_DETECTION_SAMPLE}/sample/vision_app/single_dnn/zonedetection/build/release/vision_app_zonedetection.wasm
    fi
else
    if [ ! -e $USER_WASM_FILE ]; then
        echo "Wasm file not found";
        exit 0
    fi

    WASM_FILE=$USER_WASM_FILE
fi

LOADER_DIR="${PWD}/${VNS_ENV}/testapp/build/loader"
DST_PPL_PARAMETER_FILE="$LOADER_DIR/ppl_parameter.json"
DST_OUTPUT_TENSOR_FILE="$LOADER_DIR/output_tensor.jsonc"
DST_TESTAPP_CONFIG_FILE="$LOADER_DIR/testapp_configuration.json"

NATIVE_LIBS_ARGS=" -n libdev_mock.so"

# Build Wasm
$PWD/samples/zone_detection/application/build.sh -t $LOAD_PGM $DEBUGGER 

if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
    docker build $PWD/$VNS_ENV/sdk -f $PWD/$VNS_ENV/sdk/Dockerfile -t $NAME_IMAGE --network host
fi

# "-i" or ""
INTERACTIVE_OPTION=""

MOUNT_DIRECTORY=${PWD}

PARM_ALL="-f ${WASM_FILE}${NATIVE_LIBS_ARGS} $@"

# Build test application
docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/$VNS_ENV/testapp/ && ./build.sh"

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
docker run --rm -v $MOUNT_DIRECTORY:$MOUNT_DIRECTORY --network host $INTERACTIVE_OPTION -t $NAME_IMAGE /bin/sh -c "cd $MOUNT_DIRECTORY/$VNS_ENV/testapp/ && ./run.sh $PARM_ALL"

rm -rf "$DST_PPL_PARAMETER_FILE"
rm -rf "$DST_OUTPUT_TENSOR_FILE"
rm -rf "$DST_TESTAPP_CONFIG_FILE"
