/****************************************************************************
 * Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 ****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "vision_app_public.h"

#include "analyzer_switch_dnn.h"

#include "defines.h"
#include "controllers.h"
#include "device_model.h"
#include "app_model.h"
#include "app_sequence.h"

/* prevent libc func with multi thread */
pthread_mutex_t g_libc_mutex;

int main(int argc, char *argv[]) {
    if (pthread_mutex_init(&g_libc_mutex, NULL) != 0) {
        printf("pthread_mutex_init failed libc_mutex");
        return -1;
    }

    if (setvbuf(stdout, NULL, _IOFBF, BUFSIZ) != 0) {
        ERR_PRINTF("fail setvbuf");
        return -1;
    }

    AppSequence* app = new AppSequence();
    int32_t ret = app->Initialize();
    if (!ret) {
        app->MainLoop();
    }
    app->Finalize();
    delete app;
    pthread_mutex_destroy(&g_libc_mutex);
    return 0;
}
