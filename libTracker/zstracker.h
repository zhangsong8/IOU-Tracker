#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#pragma once 

#include "basefunction/basefunction.h"
#include "baselist/baselist.h"
//#include "osal_type.h"

#define MAX_OBJ_RESULT_NUM        50
#define MAX_TRACK_NUM             50   //support max track objs

typedef enum DETECT_Object_Status_
{
    DETECT_OBJ_STATUS_NUUL = 0,
    DETECT_OBJ_STATUS_NORMAL = 1,
    DETECT_OBJ_STATUS_NEW = 2,
    DETECT_OBJ_STATUS_DELETE = 4,
    DETECT_OBJ_STATUS_MIDDLE = 8,
}Detect_Object_Status;

typedef struct TagDetectObjInfo
{
    int   left;        //
    int   right;       //
    int   top;         //
    int   bottom;      //
    int   char_class;  //
    float fConfidence; //
}TDetectObjInfo;

typedef struct ObjectDetectionResult_
{
    TDetectObjInfo  m_DetectObj[ MAX_OBJ_RESULT_NUM ];
    int obj_count;
}ObjectDetectionResult,*PObjectDetectionResult;

typedef struct TagObjectOut_Result
{
    OSAL_INT32   iObjCode;
    OSAL_BOOL    bCap;
    OSAL_INT32   iXStart;
    OSAL_INT32   iXEnd;
    OSAL_INT32   iYStart;
    OSAL_INT32   iYEnd;
    OSAL_INT32   iCapPos;
    OSAL_INT32   iFrameIndex;
    OSAL_INT32   status;
    OSAL_INT32   iObjType;
    OSAL_FLOAT32 fConfidence;

    ObjectDetectionResult* pCurDetectResult;
}ObjectOut_Result,*PObjectOut_Result;

typedef struct TagTrackTaskMain
{
    tList             m_ObjInfoFreelist;                 //free ObjInfo buf       //1000 tObjectInfo
    tList             m_ObjInfoListFreelist;             //free ObjInfoList buf   //50 tObjectInfoList
    tList             m_ObjDetectlist;                   //use  objInfoList buf(person)

    int        iFrameIndex;                       //current frame ID

    int        m_iCurObjCode;        //detect tatal num

    //ObjectDetectionResult obj_results;
    int m_iResultNum;
    ObjectOut_Result  m_TrackResult[MAX_TRACK_NUM];      //output results
}TTrackTaskMain,*PTTrackTaskMain;

typedef struct ObjectInfoList
{
    int   iObjCode;           //object code
    int   iInfoCount;         //node count of object
    int   iDetectFailCount;   //detect fail count
    int   iMatchScore;        //Last Match Score

    int   iMaxObjWidth;       //max width from all detect Frame

    tList        tInfoList;          //object track list info
}tObjectInfoList;  

typedef struct ObjectInfo
{
    VRECT        tRectPos;           //object Rect
    TPosition    tCentroid;          //object center
    int   iMatchScore;        //match score
    float fConfidence;        //object detect confidence
    int   iObjType;           //Detect ObjType
    int   iFrameIndex;        //Current FrameIndex
    tObjectInfoList   *pMatchList;   //point track List
}tObjectInfo;  


int TrackerInit(TTrackTaskMain **ppHandleTask);
void TrackerUpObjList(TTrackTaskMain *pTask);
void TrackerObjListMatch(TTrackTaskMain* pTask,ObjectDetectionResult* obj_detection_result);
OSAL_INT32 TrackerDestroyMemory(TTrackTaskMain* pTask);

#ifdef __cplusplus
}
#endif /* __cplusplus */
