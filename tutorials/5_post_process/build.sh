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

if [ $# -eq  0 ]; then
    if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
        docker build . -f $PWD/Dockerfile -t $NAME_IMAGE --network host
    fi
    docker run --rm \
        -v $PWD/sample/:/root/postvisionapp/ \
        -v $PWD/ppl_sdk/:/root/ppl_sdk/ \
        $NAME_IMAGE:latest \
        /bin/sh -c "cd /root/postvisionapp/classification && make && cd /root/postvisionapp/objectdetection && make"
else
    ARG=$1
    if [ $ARG = "od" ]; then
        docker run --rm \
            -v $PWD/sample/:/root/postvisionapp/ \
            -v $PWD/ppl_sdk/:/root/ppl_sdk/ \
            $NAME_IMAGE:latest \
            /bin/sh -c "cd /root/postvisionapp/objectdetection && make"
    elif [ $ARG = "ic" ]; then
        docker run --rm \
            -v $PWD/sample/:/root/postvisionapp/ \
            -v $PWD/ppl_sdk/:/root/ppl_sdk/ \
            $NAME_IMAGE:latest \
            /bin/sh -c "cd /root/postvisionapp/classification && make"
    elif [ $ARG = "clean" ]; then
        docker run --rm \
            -v $PWD/sample/:/root/postvisionapp/ \
            -v $PWD/ppl_sdk/:/root/ppl_sdk/ \
            $NAME_IMAGE:latest \
            /bin/sh -c "cd /root/postvisionapp/classification && make clean && cd /root/postvisionapp/objectdetection && make clean"
    elif [ $ARG = "cleanall" ]; then
        docker run --rm \
            -v $PWD/sample/:/root/postvisionapp/ \
            -v $PWD/ppl_sdk/:/root/ppl_sdk/ \
            $NAME_IMAGE:latest \
            /bin/sh -c "cd /root/postvisionapp/classification && make clean && cd /root/postvisionapp/objectdetection && make clean"  
        docker rmi $NAME_IMAGE
    else
        echo -e "ERROR: '$ARG' is unexpected argument.\nPlease see the document."
        exit 1
    fi
fi