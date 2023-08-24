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

cd "$(dirname "$0")"

ZONE_DETECTION_SAMPLE='.devcontainer/dependencies/aitrios-sdk-zone-detection-webapp-cs/sample'
VNS_ENV='tutorials/4_prepare_application/1_develop'
# set to SDK root directory
cd ../../../

usage_exit() {
    echo "usage	: ./build.sh <Options>"
    echo ""
    echo " <Options>"
    echo "    -d : (optional) build for debugging."
    echo "    -c : (optional) clean all Wasm object."
    echo "    -C : (optional) clean all Wasm object and Docker image."
    echo ""
    echo " Note : If not specified, execute building ic and od Wasm as release build."
    echo ""
    echo " Example : ./build.sh -d"
    echo "           This means that built for debugging."
    exit 1
}

BUILD_TYPE=
APP_TYPE=
OUTPUT_TENSOR_FILE=
PPL_PARAMETER_FILE=

clean() {
    echo "clean"
    docker run --rm \
        -v $PWD/:$PWD/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/${ZONE_DETECTION_SAMPLE}/sample/vision_app/single_dnn/zonedetection && make clean --file=Makefile_VnS"
}

cleanall() {
    echo "cleanall"
    clean
    docker rmi $NAME_IMAGE
}

builddockerimage() {
    if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
        echo "builddockerimage"
        docker build $VNS_ENV/sdk -f $VNS_ENV/sdk/Dockerfile -t $NAME_IMAGE --network host
    fi
}

build_zonedetection() {
    echo "build_zonedetection"
    builddockerimage
    docker run --rm \
        -v $PWD/:$PWD/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/${ZONE_DETECTION_SAMPLE}/sample/vision_app/single_dnn/zonedetection && make ${BUILD_TYPE} --file=Makefile_VnS"
}

while getopts ":cCd:o:p:t:" OPT
do
    case $OPT in
        C)  cleanall
            exit 0 ;;
        c)  clean
            exit 0 ;;
        d)  echo "for debugging"
            BUILD_TYPE="DEBUG=1"
            ;;
        o)  
            ;;
        p)  
            ;;
        t)  
            ;;
        *) usage_exit ;;
        \?) usage_exit ;;
    esac
done

build_zonedetection
