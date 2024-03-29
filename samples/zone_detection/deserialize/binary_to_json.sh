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

NAME_IMAGE='deserialize_env:1.0.0';

cd "$(dirname "$0")"

if [ ! "$(docker image ls -q "$NAME_IMAGE")" ]; then
    docker build . -f ../../../tutorials/_common/deserialize/Dockerfile -t $NAME_IMAGE --network host
fi

docker run --rm \
    -v $PWD/:/root/deserialize/ \
    $NAME_IMAGE \
    /bin/sh -c "cd /root/deserialize && flatc --defaults-json --raw-binary --strict-json -o $3 --json $1 -- $2"
