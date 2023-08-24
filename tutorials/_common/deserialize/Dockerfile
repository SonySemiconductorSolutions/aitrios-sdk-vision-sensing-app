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

FROM alpine:3.17 AS build-stage

RUN apk --update --no-cache add build-base git cmake wget \
    && rm -rf /var/cache/apk/*
RUN cd /opt \
    && wget https://github.com/google/flatbuffers/archive/refs/tags/v23.1.21.tar.gz \
    && tar -zxvf  v23.1.21.tar.gz \
    && cd flatbuffers-23.1.21 \
    && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_STRICT_MODE=ON . \
    && make -j 4

FROM alpine:3.17 AS production-stage

COPY --from=build-stage /opt/flatbuffers-23.1.21/flatc /usr/local/bin/flatc
RUN apk --update --no-cache add libstdc++ \
    && rm -rf /var/cache/apk/*
