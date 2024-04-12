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

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdio.h>
#include <pthread.h>

extern pthread_mutex_t g_libc_mutex;

/* -------------------------------------------------------- */
/* macro define                                             */
/* -------------------------------------------------------- */
#define ERR_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "E [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define WARN_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stderr, "W [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define INFO_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);fprintf(stdout, "I [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define DBG_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "D [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define VER_PRINTF(fmt, ...) pthread_mutex_lock(&g_libc_mutex);printf( "V [VisionAPP] " fmt "\n", ##__VA_ARGS__);pthread_mutex_unlock(&g_libc_mutex);
#define MUTEX_LOCK() pthread_mutex_lock(&g_libc_mutex);
#define MUTEX_UNLOCK() pthread_mutex_unlock(&g_libc_mutex);

#endif // _DEFINES_H_
