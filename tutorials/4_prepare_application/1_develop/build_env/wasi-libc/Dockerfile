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

FROM ubuntu:20.04

RUN echo "6" > /root/except_inputs.txt
RUN echo "79" >> /root/except_inputs.txt
RUN cd /root \
 && apt-get update \
 && apt-get install -y expect < except_inputs.txt \
 && apt-get install -y \
    cmake \
    g++ \
    gcc \
    git \
    libc-dev \
    make \
    ninja-build \
    patch \
    python3 \
    unzip \
    wget \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
RUN cd /opt \
 && git clone --recursive https://github.com/WebAssembly/wasi-sdk.git \
 && cd wasi-sdk \
 && git checkout wasi-sdk-19 \
 && git submodule update
RUN cd /opt/wasi-sdk \
 && sed -i -e '/EXTRA_CFLAGS\ ?=\ -O2\ -DNDEBUG/a EXTRA_CFLAGS\ +=\ -DUSE_LOCKS' src/wasi-libc/Makefile \
 && sed -i -e '/#define\ ULONG_MAX\ (2UL\*LONG_MAX+1)/a #define\ USE_LOCKS\ 1' src/wasi-libc/expected/wasm32-wasi/predefined-macros.txt
RUN cd /opt/wasi-sdk \
 && NINJA_FLAGS=-v make package
