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
. /usr/local/share/nvm/nvm.sh

DIR="build"

cd /home/vscode/VoTT-2.2.0
nvm use

# build VoTT app
if [ ! -d $DIR ];then
  npm run build
fi

# launch the server
serve -s build -l 3000 &

# wait up to 30 seconds for the server to come up
connection=false
start_time=`date +%s`
while [ $(( $(date +%s) - 30 )) -lt $start_time ];
do
  if timeout 1 bash -c 'cat < /dev/null > /dev/tcp/127.0.0.1/3000' > /dev/null 2>&1;then
    connection=true
    break
  fi
done

# when the server is up, start the VoTT app
if "${connection}" > /dev/null 2>&1;then
  npm run electron-start
else
  echo "Connection timed out."
fi