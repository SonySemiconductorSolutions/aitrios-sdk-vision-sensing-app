#!/bin/sh
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

set -e

# Download cvat if needed.
CVAT_DIR=".devcontainer/cvat/cvat"
if [ ! -d $CVAT_DIR ]
then
  echo "downloading cvat."
  cd .devcontainer/cvat/
  git clone --depth 1 https://github.com/opencv/cvat -b v2.6.1
  echo "downloading cvat completed."
else
  echo "cvat already downloaded."
fi
