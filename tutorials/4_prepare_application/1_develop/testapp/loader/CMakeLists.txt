# This product contains a modified portion of wasm-micro-runtime, which can be obtained at:
#   https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/samples/basic/CMakeLists.txt

############################################################################
# Copyright 2022-2023 Sony Semiconductor Solutions Corp. All rights reserved.
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

# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required (VERSION 2.8)

if (NOT WAMR_BUILD_PLATFORM STREQUAL "windows")
  project (loader)
else()
  project (loader C ASM)
  enable_language (ASM_MASM)
endif()

################  runtime settings  ################
string (TOLOWER ${CMAKE_HOST_SYSTEM_NAME} WAMR_BUILD_PLATFORM)
if (APPLE)
  add_definitions(-DBH_PLATFORM_DARWIN)
endif ()

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# WAMR features switch

# Set WAMR_BUILD_TARGET, currently values supported:
# "X86_64", "AMD_64", "X86_32", "AARCH64[sub]", "ARM[sub]", "THUMB[sub]",
# "MIPS", "XTENSA", "RISCV64[sub]", "RISCV32[sub]"

set (WAMR_BUILD_TARGET "X86_32")

if (NOT DEFINED WAMR_BUILD_TARGET)
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)")
    set (WAMR_BUILD_TARGET "AARCH64")
  elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
    set (WAMR_BUILD_TARGET "RISCV64")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Build as X86_64 by default in 64-bit platform
    set (WAMR_BUILD_TARGET "X86_64")
  else ()
    # Build as X86_32 by default in 32-bit platform
    set (WAMR_BUILD_TARGET "X86_32")
  endif ()
endif ()

if (NOT CMAKE_BUILD_TYPE)
  # set (CMAKE_BUILD_TYPE Release)
  set (CMAKE_BUILD_TYPE Debug)
endif ()

set (WAMR_BUILD_INTERP 1)
set (WAMR_BUILD_FAST_INTERP 1)
set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_LAZY_JIT 0)
# set (WAMR_BUILD_JIT 0)
set (WAMR_BUILD_LIBC_BUILTIN 1)
set (WAMR_BUILD_LIB_PTHREAD 1)
set (WAMR_BUILD_SIMD 1)
set (WAMR_BUILD_MINI_LOADER 0)
set (WAMR_BUILD_APP_FRAMEWORK 0)
set (WAMR_BUILD_APP_LIST WAMR_APP_BUILD_BASE)

# set (WAMR_BUILD_DUMP_CALL_STACK 1)
set (WAMR_BUILD_MEMORY_PROFILING 1)

if (NOT MSVC)
  set (WAMR_BUILD_LIBC_WASI 1)
endif ()

if (NOT MSVC)
  # linker flags
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -fPIE")
  if (NOT (CMAKE_C_COMPILER MATCHES ".*clang.*" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang"))
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
  endif ()
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
  if (WAMR_BUILD_TARGET MATCHES "X86_.*" OR WAMR_BUILD_TARGET STREQUAL "AMD_64")
    if (NOT (CMAKE_C_COMPILER MATCHES ".*clang.*" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang"))
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mindirect-branch-register")
    endif ()
  endif ()
endif ()

# for debug
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -D_FILE_OFFSET_BITS=64")
set (WAMR_BUILD_DEBUG_INTERP 1)

if (WAMR_BUILD_DEBUG_INTERP EQUAL 1)
  set (WAMR_BUILD_FAST_INTERP 0)
  set (WAMR_BUILD_MINI_LOADER 0)
  set (WAMR_BUILD_SIMD 0)
endif ()

# build out vmlib
set (WAMR_ROOT_DIR /opt/wasm-micro-runtime)
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

add_library(vmlib SHARED ${WAMR_RUNTIME_LIB_SOURCE})
set_target_properties (vmlib PROPERTIES OUTPUT_NAME vmlib)

################  application related  ################
include_directories(${CMAKE_CURRENT_LIST_DIR}/src ../../sdk/vision_app_sdk/include ../../sdk/third_party/include/parson)
include (${SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)

add_executable (loader src/main.c src/loader.c src/ppl_if.c ../../sdk/third_party/parson/parson.c ${UNCOMMON_SHARED_SOURCE})

set_target_properties (loader PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (APPLE)
  target_link_libraries (loader vmlib ${LLVM_AVAILABLE_LIBS} ${UV_A_LIBS} -lm -ldl -lpthread)
else ()
  target_link_libraries (loader vmlib ${LLVM_AVAILABLE_LIBS} ${UV_A_LIBS} -lm -ldl -lpthread -lrt)
endif ()

target_include_directories(loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library (dev_mock SHARED mock/dev_mock_if.c mock/dev_mock.cpp ../../sdk/third_party/parson/parson.c)
