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

cd "$(dirname "$0")"

usage_exit() {
    echo "usage	: ./build.sh <Options>"
    echo ""
    echo " <Options>"
    echo "    -t : (optional) specify build Wasm type. value is [ic|od]."
    echo "    -d : (optional) build for debugging."
    echo "    -c : (optional) clean all Wasm object."
    echo "    -C : (optional) clean all Wasm object and Docker image."
    echo ""
    echo " Note : If not specified, execute building ic and od Wasm as release build."
    echo ""
    echo " Example : ./build.sh -d -t od"
    echo "           This means that building type is od and built for debugging."
    exit 1
}

BUILD_TYPE=
APP_TYPE=
OUTPUT_TENSOR_FILE=
PPL_PARAMETER_FILE=

clean() {
    echo "clean"
    docker run --rm \
        -v $PWD/sdk/:$PWD/sdk/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/sdk/sample && make clean"
}

cleanall() {
    echo "cleanall"
    clean
    docker rmi $NAME_IMAGE
}

builddockerimage() {
    if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
        echo "builddockerimage"
        docker build ./sdk -f $PWD/sdk/Dockerfile -t $NAME_IMAGE --network host
    fi
}

build_ic_od() {
    echo "build_ic_od"
    builddockerimage
    docker run --rm \
        -v $PWD/sdk/:$PWD/sdk/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/sdk/sample && make ${BUILD_TYPE}"
}

build_ic() {
    echo "build_ic"
    builddockerimage
    docker run --rm \
        -v $PWD/sdk/:$PWD/sdk/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/sdk/sample && make ${BUILD_TYPE} APPTYPE=ic"
}

build_od() {
    echo "build_od"
    builddockerimage
    docker run --rm \
        -v $PWD/sdk/:$PWD/sdk/ \
        $NAME_IMAGE \
        /bin/sh -c "cd ${PWD}/sdk/sample && make ${BUILD_TYPE} APPTYPE=od"
}

# while getopts ":AaCcdhlnpsvu-:" OPT
while getopts ":cCdt:o:p:" OPT
do
    case $OPT in
        C)  cleanall
            exit 0 ;;
        c)  clean
            exit 0 ;;
        d)  echo "for debugging"
            BUILD_TYPE="DEBUG=1"
            ;;
        t)  
            if [ "$OPTARG" = "ic" ]; then
                APP_TYPE=$OPTARG
            elif [ "$OPTARG" = "od" ]; then
                APP_TYPE=$OPTARG
            else
                echo "-t options must be ic or od."
                exit 0
            fi;
            ;;
        o)  
            ;;
        p)  
            ;;
        *) usage_exit ;;
        \?) usage_exit ;;
    esac
done

if [ "$APP_TYPE" = "od" ]; then
    build_od
elif [ "$APP_TYPE" = "ic" ]; then
    build_ic
else
    build_ic_od
fi;
