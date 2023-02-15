#!/bin/bash
# Copyright 2022 Sony Semiconductor Solutions Corp. All rights reserved.
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

NAME_IMAGE='ppl_build_env';

cd "$(dirname "$0")"

usage_exit() {
    echo "usage	: ./start.sh <Options>"
    echo ""
    echo " <Options>"
    echo "    -t : (mandatory) specify build/run Wasm type. value is [ic|od|param_debug]."
    echo "    -d : (optional) build for debugging. start and wait for attaching debugger."
    echo "    -o : (optional) specify output tensor jsonc file for debugging. value is absolute path to the file. The file must be in ./tutorials/4_prepare_application/1_develop/**/*."
    echo "    -p : (optional) specify ppl parameter json file for debugging. value is absolute path to the file. The file must be in ./tutorials/4_prepare_application/1_develop/**/*."
    echo ""
    echo " Example : ./start.sh -d -t od"
    echo "           means that Wasm type is od and built/ran for debugging."
    exit 1
}

LOAD_PGM=none
DEBUGGER=""

while getopts :do:p:t: option
do
  case $option in
    d)
      DEBUGGER="-d";;
    o);;
    p);;
    t)
      LOAD_PGM=${OPTARG};;
    ?);;
  esac
done

if [ $LOAD_PGM != "ic" ] && [ $LOAD_PGM != "od" ] && [ $LOAD_PGM != "param_debug" ]; then
    usage_exit
fi

# Switching Wasm References
if [ $LOAD_PGM = "ic" ]; then
    if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
        WASM_FILE=${PWD}/sample/classification/debug/ppl_classification.wasm
    else
        WASM_FILE=${PWD}/sample/classification/release/ppl_classification.wasm
    fi
elif [ $LOAD_PGM = "od" ]; then
    if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
        WASM_FILE=${PWD}/sample/objectdetection/debug/ppl_objectdetection.wasm
    else
        WASM_FILE=${PWD}/sample/objectdetection/release/ppl_objectdetection.wasm  
    fi
else
    if [ -n "$DEBUGGER" ] && [ $DEBUGGER = "-d" ]; then
        WASM_FILE=${PWD}/sample/param_debug/debug/ppl_objectdetection.wasm
    else
        WASM_FILE=${PWD}/sample/param_debug/release/ppl_objectdetection.wasm  
    fi
fi

PARM_ALL="-f ${WASM_FILE} $@"

./build.sh -t $LOAD_PGM $DEBUGGER 

if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
    docker build . -f $PWD/Dockerfile -t $NAME_IMAGE --network host
fi

docker run --rm -v $PWD:$PWD --network host -t -i $NAME_IMAGE /bin/sh -c "cd $PWD/testapp/ && ./build.sh && ./run.sh $PARM_ALL"
