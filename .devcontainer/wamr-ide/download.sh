#!/bin/sh
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

set -e

wget https://github.com/bytecodealliance/wasm-micro-runtime/releases/download/WAMR-1.1.2/wamr-lldb-1.1.2-x86_64-ubuntu-20.04.tar.gz -P .devcontainer/wamr-ide/

wget https://github.com/bytecodealliance/wasm-micro-runtime/releases/download/WAMR-1.1.2/wamr_ide-1.1.2.tar.gz -P .devcontainer/wamr-ide/

tar -zxvf .devcontainer/wamr-ide/wamr_ide-1.1.2.tar.gz -C /home/vscode/
