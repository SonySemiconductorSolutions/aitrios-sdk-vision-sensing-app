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

ifeq ($(APPTYPE),od)
	SUBDIRS += vision_app/single_dnn/objectdetection
else
ifeq ($(APPTYPE),ic)
	SUBDIRS += vision_app/single_dnn/classification
else
ifeq ($(APPTYPE),switchdnn)
	SUBDIRS += vision_app/switch_dnn/switch_od_ic
else
	SUBDIRS += vision_app/single_dnn/objectdetection
	SUBDIRS += vision_app/single_dnn/classification
	SUBDIRS += vision_app/switch_dnn/switch_od_ic
endif
endif
endif

all:$(SUBDIRS)

clean:$(SUBDIRS)

$(SUBDIRS): FORCE
	make -C $@ $(MAKECMDGOALS)

FORCE:

