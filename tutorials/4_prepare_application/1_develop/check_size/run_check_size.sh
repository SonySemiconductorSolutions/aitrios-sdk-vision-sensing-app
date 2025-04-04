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

cd "$(dirname "$0")"

case $1 in
    *\.wasm)
        if [ -e $1 ]; then
            :
        else
            echo "Not found $1"
            exit 0
        fi        
        ;;
    *)
        echo "Specify Wasm file for checking size."
        exit 0
        ;;
esac

echo "********************************************************************************"
echo "Check size for `basename $1`"
echo "********************************************************************************"
aot=${PWD}/`basename $1 .wasm`.aot 
./wamrc --target=xtensa --enable-multi-thread --size-level=0 -o $aot $1
python ./check_aot_size.py $aot
rm $aot