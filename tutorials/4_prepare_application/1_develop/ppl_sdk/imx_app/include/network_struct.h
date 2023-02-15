/****************************************************************************
 * Copyright 2022 Sony Semiconductor Solutions Corp. All rights reserved.
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

#ifndef _NETWORK_STRUCT_H_
#define _NETWORK_STRUCT_H_


/* -------------------------------------------------------- */
/* structure                                                */
/* -------------------------------------------------------- */
typedef struct tagPPL_Bbox {
    uint16_t    m_xmin;
    uint16_t    m_ymin;
    uint16_t    m_xmax;
    uint16_t    m_ymax;
} PPL_Bbox;

typedef struct tagPPL_Point {
    uint32_t    m_x;
    uint32_t    m_y;
} PPL_Point;

/* Classification */
typedef struct tagPPL_ClassificationItem {
    int         m_class;
    float       m_score;
} PPL_ClassificationItem;

typedef struct tagPPL_ClassificationData {
    PPL_ClassificationItem  *mp_class_items;
} PPL_ClassificationData;

/* Object Detection */
typedef struct tagPPL_ObjectDetectionData {
    uint8_t     m_detected_num;
    uint8_t     m_dmy[3];
    PPL_Bbox    *mp_box;
    float       *mp_scores;
    uint8_t     *mp_classes;
} PPL_ObjectDetectionData;

/* Key Point */
typedef struct tagPPL_Keypoint {
    PPL_Point   *mp_points;
} PPL_Keypoint;

typedef struct tagPPL_KeypointData {
    uint8_t         m_detected_num;
    uint8_t         m_dmy[3];
    PPL_Bbox        *mp_box;
    float           *mp_scores;
    PPL_Keypoint    *mp_keypoints;
} PPL_KeypointData;


#endif /* _NETWORK_STRUCT_H_ */