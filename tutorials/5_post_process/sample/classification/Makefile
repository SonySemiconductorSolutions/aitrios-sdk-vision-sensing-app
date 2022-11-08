############################################################################
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
#
############################################################################

# Non-customizable area
BINPATH=/opt/wasi-sdk/bin
LIBPATH=/root/ppl_sdk
CC        = $(BINPATH)/clang
CPP       = $(BINPATH)/clang++
CFLAGS    = --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
            -I $(LIBPATH)/imx_app/include \
            -I $(LIBPATH)/lib \
            -I $(LIBPATH)/lib/parson/include \
            -I ./include
CPPFLAGS := $(CFLAGS)
LDFLAGS   = --sysroot=/opt/wasi-sdk/share/wasi-sysroot/ \
            -Wl,--export=PPL_Initialize \
            -Wl,--export=PPL_Analyze \
            -Wl,--export=PPL_ResultRelease \
            -Wl,--export=PPL_Finalize \
            -Wl,--export=PPL_GetPplVersion \
            -Wl,--no-entry \
            -Wl,--allow-undefined \
            -Wl,--export=malloc \
            -Wl,--export=free \
            -Wl,--export=__heap_base \
            -Wl,--export=__data_end

# Customizable area
USER_CFLAGS    = -O2
USER_CPPFLAGS  = $(USER_CFLAGS)
USER_LDFLAGS   = -z stack-size=8192 \
                 -Wl,--initial-memory=65536
CFLAGS        += $(USER_CFLAGS)
CPPFLAGS      += $(USER_CPPFLAGS)
LDFLAGS       += $(USER_LDFLAGS)
CPPOBJS        = ppl_classification.o \
                 $(LIBPATH)/lib/parson/parson.o
TARGET         = ppl_classification

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

ALL: $(CPPOBJS)
	$(CPP) $(LDFLAGS) -o $(TARGET).wasm $(CPPOBJS)

clean:
	rm -rf *.o *.wasm
	find $(LIBPATH)/ -name "*.o" -type f -delete
