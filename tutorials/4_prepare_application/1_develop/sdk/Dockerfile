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

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
       make \
       libxml2 \
       wget \
       cmake \
       build-essential \
       libc6-dev-i386 gcc-multilib g++-multilib \
       libxml2-dev libedit-dev git build-essential cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
RUN cd /opt \
    && wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-19/wasi-sdk-19.0-linux.tar.gz \
    && tar zxvf wasi-sdk-19.0-linux.tar.gz \
    && ln -s wasi-sdk-19.0 wasi-sdk \
    && rm wasi-sdk-19.0-linux.tar.gz
RUN cd /opt/wasi-sdk/share/wasi-sysroot/lib/wasm32-wasi \
    && mv crt1-command.o crt1-command.o.bak
WORKDIR /root
COPY third_party/include/wasm-micro-runtime/pthread.h /opt/wasi-sdk/share/wasi-sysroot/include
COPY third_party/lib/libc_for_pthread.a /opt/wasi-sdk/share/wasi-sysroot/lib/wasm32-wasi/libc.a

RUN cd /opt \
    && wget https://github.com/google/flatbuffers/archive/refs/tags/v23.1.21.tar.gz \
    && tar -zxvf  v23.1.21.tar.gz \
    && cd flatbuffers-23.1.21 \
    && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_STRICT_MODE=ON . \
    && make -j 4 \
    && make install

# Get wasm-micro-runtime source code for building test app
RUN cd /opt \
    && git clone https://github.com/bytecodealliance/wasm-micro-runtime.git -b WAMR-1.1.2