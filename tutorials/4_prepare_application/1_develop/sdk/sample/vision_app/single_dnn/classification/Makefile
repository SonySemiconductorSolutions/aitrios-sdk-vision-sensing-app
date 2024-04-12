############################################################################
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
#
############################################################################

DEBUG ?= 0
TESTAPP_DUMP_MEMORY_CONSUMPTION ?= 0

CLEAN_DIR = $(PWD)/build
DEBUG_BUILD_DIR = $(PWD)/build/debug/sample/vision_app/single_dnn/classification
RELEASE_BUILD_DIR = $(PWD)/build/release/sample/vision_app/single_dnn/classification
ifeq ($(DEBUG), 1)
	BUILD_DIR = $(DEBUG_BUILD_DIR)
	WASM_OUT_DIR = $(PWD)/build/debug
else
	BUILD_DIR = $(RELEASE_BUILD_DIR)
	WASM_OUT_DIR = $(PWD)/build/release
endif

# Non-customizable area
BINPATH=/opt/wasi-sdk/bin
THIRD_PARTY_PATH=../../../../third_party
CC        = $(BINPATH)/clang
CPP       = $(BINPATH)/clang++
CFLAGS    = --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
            -I ./include \
            -I ../../../../vision_app_sdk/include \
            -I $(THIRD_PARTY_PATH)/include \
            -I $(THIRD_PARTY_PATH)/include/parson
CPPFLAGS := $(CFLAGS)
LDFLAGS   = --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
            -pthread \
            -Wl,--shared-memory \
            -Wl,--no-check-features \
            -Wl,--no-entry \
            -Wl,--allow-undefined \
            -Wl,--export=malloc \
            -Wl,--export=free \
            -Wl,--export=__heap_base \
            -Wl,--export=__data_end

# Customizable area
ifeq ($(DEBUG), 1)
	USER_CFLAGS    = -g -O0
else
	USER_CFLAGS    = -O2
endif
ifeq ($(TESTAPP_DUMP_MEMORY_CONSUMPTION), 1)
	USER_CFLAGS    += -DTESTAPP_DUMP_MEMORY_CONSUMPTION
endif
USER_CPPFLAGS  = $(USER_CFLAGS)
USER_LDFLAGS   = -z stack-size=32768 \
                 -Wl,--initial-memory=65536,--max-memory=2097152
CFLAGS        += $(USER_CFLAGS)
CPPFLAGS      += $(USER_CPPFLAGS) \
                -I ../../../../sample/include/classification \
                -I ../../../../sample/post_process/classification/include
LDFLAGS       += $(USER_LDFLAGS)
CPPOBJS        = ./src/vision_app_classification.o \
                ../../../../sample/post_process/classification/src/analyzer_classification.o \
                $(THIRD_PARTY_PATH)/parson/parson.o
CPPOBJS_OUT    = $(BUILD_DIR)/src/vision_app_classification.o \
                $(BUILD_DIR)/../../../../sample/post_process/classification/src/analyzer_classification.o \
                $(BUILD_DIR)/$(THIRD_PARTY_PATH)/parson/parson.o
TARGET         = vision_app_classification

%.o: %.c
	mkdir -p $(BUILD_DIR)/$(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $< -o $(BUILD_DIR)/$@

%.o: %.cpp
	echo $@
	mkdir -p $(BUILD_DIR)/$(dir $@)
	$(CPP) $(CPPFLAGS) -o $@ -c $< -o $(BUILD_DIR)/$@

ALL: $(CPPOBJS)
	mkdir -p $(BUILD_DIR)
	$(CPP) $(LDFLAGS) -o $(WASM_OUT_DIR)/$(TARGET).wasm $(CPPOBJS_OUT)

clean:
	rm -rf $(CLEAN_DIR)

