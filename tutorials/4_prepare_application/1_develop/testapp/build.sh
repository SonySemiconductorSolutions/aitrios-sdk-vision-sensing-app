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

echo -e $PWD "testapp build.sh pwd"

echo "#### build test app ####"

LOADER_SRC=${PWD}/loader

usage_exit() {
	echo "Usage: $0 [-j]" 1>&2
	echo "  -j: enable JIT"
	exit 1
}

build_args=()
while getopts jdh OPT
do
    case $OPT in
        j) build_args+=("-DWAMR_BUILD_JIT=1")
            ;;
        h)  usage_exit
            ;;
        \?) usage_exit
            ;;
    esac
done

rm -rf build/loader
mkdir -p build/loader
cd build/loader
cmake ${LOADER_SRC} ${build_args[@]}
make -j ${nproc}
