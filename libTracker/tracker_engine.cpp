/**
 * ============================================================================
 *
 * Copyright (C) 2019, Huawei Technologies Co., Ltd. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1 Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   2 Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   3 Neither the names of the copyright holders nor the names of the
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */
#include "tracker_engine.h"
#include "error_code.h"
#include "hiaiengine/ai_memory.h"
#include "hiaiengine/c_graph.h"
#include "hiaiengine/data_type.h"
#include "hiaiengine/log.h"
#include <memory>
#include <opencv2/opencv.hpp>

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

HIAI_REGISTER_DATA_TYPE("StreamInfo", StreamInfo);
HIAI_REGISTER_DATA_TYPE("FrameData", FrameData);
HIAI_REGISTER_DATA_TYPE("FrameDetectInfo", FrameDetectInfo);
HIAI_REGISTER_DATA_TYPE("DeviceStreamData", DeviceStreamData);
HIAI_REGISTER_DATA_TYPE("DeviceStreamBatchData", DeviceStreamBatchData);

HIAI_StatusT TrackerEngine::Init(const hiai::AIConfig &config,
                                         const std::vector<hiai::AIModelDescription> &model_desc)
{
    for (int i = 0; i < TASK_BATCH_NUM; i++)
    {
        TrackerInit(&pTask[i]);
    }
    return HIAI_OK;
}

TrackerEngine::~TrackerEngine()
{
    for (int i = 0; i < TASK_BATCH_NUM; i++)
    {
        TrackerDestroyMemory(pTask[i]);
    }
}

HIAI_IMPL_ENGINE_PROCESS("TrackerEngine", TrackerEngine, TRACKER_INPUT_SIZE)
{
    HIAI_ENGINE_LOG(APP_ERROR, "TrackerEngine Process start");
    HIAI_StatusT ret = HIAI_OK;

    std::shared_ptr<DeviceStreamBatchData> inputArg = std::static_pointer_cast<DeviceStreamBatchData>(arg0);
    inputArgQueue = inputArg->batchData;
    for (int n = 0; n < inputArgQueue.size(); n++) {
        std::shared_ptr<DeviceStreamData> one_inputArg = inputArgQueue[n];
        ObjectDetectionResult obj_detection_result;
        obj_detection_result.obj_count = one_inputArg->detectResult.size();

        for (int j = 0; j < one_inputArg->detectResult.size(); j++)
        {
            obj_detection_result.m_DetectObj[j].left = one_inputArg->detectResult[j].location.anchor_lt.x;
            obj_detection_result.m_DetectObj[j].top = one_inputArg->detectResult[j].location.anchor_lt.y;
            obj_detection_result.m_DetectObj[j].right = one_inputArg->detectResult[j].location.anchor_rb.x;
            obj_detection_result.m_DetectObj[j].bottom = one_inputArg->detectResult[j].location.anchor_rb.y;
            obj_detection_result.m_DetectObj[j].char_class = one_inputArg->detectResult[j].id;
            obj_detection_result.m_DetectObj[j].fConfidence = one_inputArg->detectResult[j].confidence;
        }
        for (int i = 0; i < obj_detection_result.obj_count - 1; ++i)
        {
            int aleft = obj_detection_result.m_DetectObj[i].left;
            int aright = obj_detection_result.m_DetectObj[i].right;
            int abottom = obj_detection_result.m_DetectObj[i].bottom;
            int atop = obj_detection_result.m_DetectObj[i].top;
            float invalue, onvalue;
            for (int j = i + 1; j < obj_detection_result.obj_count; ++j)
            {
                int bleft = obj_detection_result.m_DetectObj[j].left;
                int bright = obj_detection_result.m_DetectObj[j].right;
                int bbottom = obj_detection_result.m_DetectObj[j].bottom;
                int btop = obj_detection_result.m_DetectObj[j].top;
                if (aleft > bright || bleft > aright || bbottom < atop || abottom < btop)
                {
                    continue;
                }
                invalue = ((aleft > bleft ? aleft : bleft) - (aright < bright ? aright : bright)) * ((atop > btop ? atop : btop) - (abottom < bbottom ? abottom : bbottom));
                onvalue = (aright - aleft) * (abottom - atop) + (bright - bleft) * (bbottom - btop) - invalue;
                if (((invalue * 1.0) / onvalue) > 0.5)
                {
                    if (obj_detection_result.m_DetectObj[i].fConfidence > obj_detection_result.m_DetectObj[j].fConfidence)
                    {
                        if (j != obj_detection_result.obj_count - 1)
                        {
                            for (int k = j + 1; k < obj_detection_result.obj_count; ++k)
                            {
                                obj_detection_result.m_DetectObj[k - 1] = obj_detection_result.m_DetectObj[k];
                            }
                        }
                        else {
                            ;
                        }
                        j--;
                        obj_detection_result.obj_count--;
                    }
                    else
                    {
                        for (int k = i + 1; k < obj_detection_result.obj_count; ++k)
                        {
                            obj_detection_result.m_DetectObj[k - 1] = obj_detection_result.m_DetectObj[k];
                        }
                        i--;
                        j--;
                        obj_detection_result.obj_count--;
                        break;
                    }
                }
            }
        }
        //HIAI_ENGINE_LOG(APP_ERROR, "a TrackerEngine Process start %d   %d", i, one_inputArg->detectResult.size());
        if (obj_detection_result.obj_count > 0)
        {
            TrackerObjListMatch(pTask[n], &obj_detection_result);
        }
        //HIAI_ENGINE_LOG(APP_ERROR, "b TrackerEngine Process start %d", i);
        TrackerUpObjList(pTask[n]);
        //HIAI_ENGINE_LOG(APP_ERROR, "c TrackerEngine Process start %d", i);

        //vector<FrameDetectInfo>().swap((one_inputArg->detectResult));
        one_inputArg->detectResult.clear();
        for (int k = 0; k < pTask[n]->m_iResultNum; k++)
        {
            if (pTask[n]->m_TrackResult[n].status & DETECT_OBJ_STATUS_NORMAL)
            {
            }
            else {
                continue;
            }
            //= pTask->m_TrackResult[k].;
            FrameDetectInfo temp_tracker_result;
            temp_tracker_result.confidence = pTask[n]->m_TrackResult[k].fConfidence;
            temp_tracker_result.id = pTask[n]->m_TrackResult[k].iObjCode;
            temp_tracker_result.type = pTask[n]->m_TrackResult[k].iObjType;
            temp_tracker_result.location.anchor_lt.x = pTask[n]->m_TrackResult[k].iXStart;
            temp_tracker_result.location.anchor_lt.y = pTask[n]->m_TrackResult[k].iYStart;
            temp_tracker_result.location.anchor_rb.x = pTask[n]->m_TrackResult[k].iXEnd;
            temp_tracker_result.location.anchor_rb.y = pTask[n]->m_TrackResult[k].iYEnd;
            one_inputArg->detectResult.push_back(temp_tracker_result);
        } 
    }
    HIAI_ENGINE_LOG(APP_ERROR, "d TrackerEngine Process start ");

    std::shared_ptr<DeviceStreamBatchData> sendDataToDemuxFromTracker = inputArg;
    if (HIAI_OK != hiai::Engine::SendData(0, "DeviceStreamBatchData",
        std::static_pointer_cast<void>(sendDataToDemuxFromTracker))) {
        HIAI_ENGINE_LOG(APP_ERROR, "TrackerEngine senddata error!");
        return HIAI_ERROR;
    }
    else {
        HIAI_ENGINE_LOG(APP_INFO, "TrackerEngine senddata successfully!");
    }
    return HIAI_OK;
}
