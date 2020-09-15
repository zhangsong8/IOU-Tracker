#include "zstracker.h"
#include <stdbool.h>
//////////////////////////////////private function////////////////////////////////////////

/***************************************************************
funcName  : ObjectNodeInit
funcs	  : init list
Param In  : num--node num; pList--List; iDataLength--node data size;
Prame Out :
return    : 0 -- success  other -- fail
**************************************************************/
static int ObjectNodeInit(int num,tList* pList,int iDataLength)
{
    int i = 0;
    void *pNode = NULL;
    for (i = 0; i < num; i++)
    {
        MEMO_MALLOC(pNode,OSAL_UCHAR,iDataLength);
        if (pNode != NULL)
        {
            listnodeAdd(pList, pNode);
            pNode = NULL;
        }
        else
        {
            return OSAL_FAIL;
        }
    }
    return OSAL_OK;
}
/***************************************************************
funcName  : ObjectNodeQuit
funcs	  : destroy list
Param In  : pList--List;
Prame Out :
return    : 0 -- success  other -- fail
**************************************************************/
static int ObjectNodeQuit(tList* pList)
{
    void *pNode = NULL;

    while(1)
    {
        pNode = listPop(pList);

        if (pNode != NULL)
        {
            MEMO_FREE(pNode);
            pNode = NULL;
        }
        else
        {
            break;
        }
    }
    return OSAL_OK;
}

//push to list
static int ObjectNodePush(void* pNode,tList* pList)
{
    listnodeAdd(pList, pNode);
    return OSAL_OK;
}
//pop from list

static void* ObjectNodePop(tList* pList)
{
    void *pNode = NULL;

    pNode = listPop(pList);

    return pNode;
}

//restore node from SrcList to DstList
static void ObjectNodeRestore(tList* pSrcList,tList* pDstList)
{
    void *pNode = NULL;
    while (1)
    {
        pNode = ObjectNodePop(pSrcList);

        if (pNode)
        {
            ObjectNodePush(pNode,pDstList);
        }
        else
        {
            break;
        }
    }
}
static void ObjectDetectFree(tList *DetectList,tList *InfoListFree,tList *InfoFree)
{
    tObjectInfoList *pNode = NULL;
    struct listnode *node = NULL;
    void *data = NULL;
    //restore all info
    LIST_LOOP(DetectList,data,node)
    {
        pNode = (tObjectInfoList*)data;
        ObjectNodeRestore(&pNode->tInfoList,InfoFree);
    }
    //restore all infoList
    ObjectNodeRestore(DetectList,InfoListFree);
}
/***************************************************************
funcName  : Tr_MatchObjInfo
funcs	  : match objInfo
Param In  : pSrcObjInfo--pSrcObj; pObjInfo--pDstObj;
Prame Out :
return    : match score
**************************************************************/
//compute IOU
static OSAL_FLOAT32 OverLap(OSAL_FLOAT32 x1,OSAL_FLOAT32 x2,OSAL_FLOAT32 y1,OSAL_FLOAT32 y2)
{
    OSAL_FLOAT32 left = (x1>y1?x1:y1);
    OSAL_FLOAT32 right = (x2<y2?x2:y2);

    return right-left;
}

static float box_intersection(VRECT *a,VRECT *b)
{
    float w = OverLap(a->left,a->right,b->left,b->right);
    float h = OverLap(a->top,a->bottom,b->top,b->bottom);

    if (w < 0 || h < 0)
    {
        return 0.0;
    }
    else
    {
        return (w*h);
    }
}
static float GetRectIOU(VRECT *a,VRECT *b)
{
    float InValue = box_intersection(a,b);
    float UnValue = ((a->right-a->left)*(a->bottom-a->top)+(b->right-b->left)*(b->bottom-b->top)-InValue);

    return InValue/UnValue;
}
static OSAL_INT32 Tr_MatchObjInfo(tObjectInfo *pSrcObjInfo,tObjectInfo *pObjInfo)
{
    OSAL_INT32 iMatchScore = 0;

    iMatchScore = 100*GetRectIOU(&pSrcObjInfo->tRectPos,&pObjInfo->tRectPos);

    return iMatchScore;
}

int TrackerInit(TTrackTaskMain **ppHandleTask)
{
    TTrackTaskMain *pTask = OSAL_NULL;
    VD_MALLOC_BIGMEM();

    MEMO_MALLOC(pTask,TTrackTaskMain,1);
    if (pTask == OSAL_NULL)
    {
        printf("pTask MALLOC error\n");
        return 1;
    }
    memset(pTask,0,sizeof(TTrackTaskMain));

    ObjectNodeInit(2000, &pTask->m_ObjInfoFreelist,     sizeof(tObjectInfo));
    ObjectNodeInit(200,   &pTask->m_ObjInfoListFreelist, sizeof(tObjectInfoList));

    pTask->m_iCurObjCode = 0;
    pTask->m_ObjDetectlist.count = 0;
    *ppHandleTask = pTask;
    return 0;
}

/***************************************************************
funcName  : DestroyMemory
funcs	  : free buf
Param In  : pTask--track task;
Prame Out :
return    : 0 -- success  other -- fail
**************************************************************/
OSAL_INT32 TrackerDestroyMemory(TTrackTaskMain* pTask)
{

    ObjectDetectFree(&pTask->m_ObjDetectlist,&pTask->m_ObjInfoListFreelist,&pTask->m_ObjInfoFreelist);
    ObjectNodeQuit(&pTask->m_ObjInfoFreelist);
    ObjectNodeQuit(&pTask->m_ObjInfoListFreelist);

    return OSAL_OK;
}


/***************************************************************
funcName  : ObjListMatch
funcs	  : match all objInfo to Lists
Param In  : pTask--task; obj_detection_result--detect results;
Prame Out :
return    :
**************************************************************/
void TrackerObjListMatch(TTrackTaskMain* pTask,ObjectDetectionResult* obj_detection_result)
{
    struct listnode *node = NULL;
    void *data = NULL;
    tObjectInfoList *pObjInfoList = NULL;
    tObjectInfo  *pObjNode = NULL;
    tObjectInfo  *pTailInfo = NULL;
    //VRECT tDetectRect;
    int obj_i;
    int iMatchIndex = -1;
    int iMatchScore = 0,iMatchScoreMax = 0;
    int iObjectFlag[MAX_OBJ_RESULT_NUM];
    //VRECT tNewRect[MAX_OBJ_RESULT_NUM];
	TDetectObjInfo* pDetectObj = NULL;
    int obj_result_num = 0;
	int iFrameIndex = 0;

    memset(iObjectFlag,0,MAX_OBJ_RESULT_NUM*sizeof(int));

    obj_result_num = obj_detection_result->obj_count;
	pDetectObj     = obj_detection_result->m_DetectObj;

	iFrameIndex = pTask->iFrameIndex;
    printf("nSel > -1%saaaaaaaaaaaaaaaaaaaaaaaaaaaa-------------------------------- %d\n",__func__,__LINE__);
//*/
    //zhangsong modify start
    //LOOP DetectList
    int src_obj_index =0 ;
    int iou_matchscore[60][60] = {0};
    //printf(" *********************%d\n",__LINE__);
    LIST_LOOP(&pTask->m_ObjDetectlist,data,node)
    {
        //printf(" *********************%d\n",__LINE__);
        pObjInfoList = (tObjectInfoList*)data;
        //get free objNode
        pObjNode = (tObjectInfo *)ObjectNodePop(&pTask->m_ObjInfoFreelist);

        if(OSAL_NULL == pObjNode)
        {
            continue;
        }

        pTailInfo = (tObjectInfo*)listnodeTail(&pObjInfoList->tInfoList);

        //to match detect object
        for( obj_i = 0; obj_i < obj_result_num; )
        {
            //get match score
            pObjNode->tRectPos.left   = pDetectObj[obj_i].left;
            pObjNode->tRectPos.right  = pDetectObj[obj_i].right;
            pObjNode->tRectPos.top    = pDetectObj[obj_i].top;
            pObjNode->tRectPos.bottom = pDetectObj[obj_i].bottom;

            iMatchScore = Tr_MatchObjInfo(pTailInfo,pObjNode);

            iou_matchscore[src_obj_index][obj_i++] = iMatchScore;
        }
        src_obj_index++;
        ObjectNodePush(pObjNode,&pTask->m_ObjInfoFreelist);
    }
    src_obj_index = 0;
    obj_i = 0;
    bool lsmaxscore = true;
    //printf(" *********************%d\n",__LINE__);
    if(NULL != (struct listnode *)(pTask->m_ObjDetectlist.head))
    {
        //printf(" *********************%d\n",__LINE__);
        for(src_obj_index = 0;src_obj_index < pTask->m_ObjDetectlist.count ; src_obj_index++)
        {
            //printf(" ******%d***************%d\n",pTask->m_ObjDetectlist.count,__LINE__);
            iMatchIndex = -1;
            iMatchScoreMax = 0;
            for( obj_i = 0; obj_i < obj_result_num; obj_i++)
            {
                if(iou_matchscore[src_obj_index][obj_i] > iMatchScoreMax)
                {
                    iMatchScoreMax = iou_matchscore[src_obj_index][obj_i];
                    iMatchIndex = obj_i;
                }
                //printf(" ******%d*************%d**%d\n",iou_matchscore[src_obj_index][obj_i],iMatchIndex,iMatchScoreMax);
            }
            lsmaxscore = true;
            for(int j = 0;j < pTask->m_ObjDetectlist.count ; j++)
            {
                //printf(" *********************%d\n",__LINE__);
                if(iou_matchscore[j][iMatchIndex] > iMatchScoreMax)
                {
                    //printf(" *********************%d\n",__LINE__);
                    lsmaxscore = false;
                    break;
                }else{
                    continue;
                }
            }
            if(lsmaxscore == true)
            {
                //printf(" *********************%d\n",__LINE__);
                pObjNode = (tObjectInfo *)ObjectNodePop(&pTask->m_ObjInfoFreelist);
                if(iMatchScoreMax > 0.05)
                {
                    //printf(" *********************%d\n",__LINE__);
                    iObjectFlag[iMatchIndex] = 1;
                    struct listnode *temp_node = pTask->m_ObjDetectlist.head;
                    for(int i = 0 ; i < src_obj_index ; i++)
                    {
                       temp_node =  temp_node->next;
                    }
                    pObjInfoList = (tObjectInfoList *)temp_node->data;

                    pObjInfoList->iDetectFailCount = 0;
                    pObjInfoList->iMatchScore  = iMatchScoreMax;
                    pObjInfoList->iInfoCount++;

                    pObjInfoList->iMaxObjWidth = MAX(pObjInfoList->iMaxObjWidth,pObjNode->tRectPos.right - pObjNode->tRectPos.left);

                    pObjNode->tRectPos.left   = pDetectObj[iMatchIndex].left;
                    pObjNode->tRectPos.right  = pDetectObj[iMatchIndex].right;
                    pObjNode->tRectPos.top    = pDetectObj[iMatchIndex].top;
                    pObjNode->tRectPos.bottom = pDetectObj[iMatchIndex].bottom;
                    pObjNode->tCentroid.x = (pObjNode->tRectPos.left + pObjNode->tRectPos.right)/2;
                    pObjNode->tCentroid.y = (pObjNode->tRectPos.top  + pObjNode->tRectPos.bottom)/2;
                    pObjNode->iFrameIndex = iFrameIndex;

                    pObjNode->iMatchScore = iMatchScoreMax;
                    pObjNode->fConfidence = pDetectObj[ iMatchIndex ].fConfidence;
                    pObjNode->iObjType   = pDetectObj[ iMatchIndex ].char_class;

            pObjNode->pMatchList = pObjInfoList;

                    ObjectNodePush(pObjNode,&pObjInfoList->tInfoList);
                }
                else
                {
                    ObjectNodePush(pObjNode,&pTask->m_ObjInfoFreelist);
                }

            }else{
                continue;
            }
        }
    }
    //zhangsong modify end
    //printf(" *********************%d\n",__LINE__);
    for( obj_i = 0; obj_i < obj_result_num;obj_i++ )
    {
        if(iObjectFlag[obj_i] == 1)
        {
            continue;
        }
        printf(" *********************%d\n",__LINE__);
        //get free info
        pObjNode = (tObjectInfo *)ObjectNodePop(&pTask->m_ObjInfoFreelist);
        memset(pObjNode, 0, sizeof(tObjectInfo));

        if(NULL == pObjNode)
        {
            continue;
        }
        //detect result is pos in srcImg
        pObjNode->tRectPos.left   = pDetectObj[obj_i].left;
        pObjNode->tRectPos.right  = pDetectObj[obj_i].right;
        pObjNode->tRectPos.top    = pDetectObj[obj_i].top;
        pObjNode->tRectPos.bottom = pDetectObj[obj_i].bottom;

        pObjNode->tCentroid.x = (pObjNode->tRectPos.left + pObjNode->tRectPos.right)/2;
        pObjNode->tCentroid.y = (pObjNode->tRectPos.top  + pObjNode->tRectPos.bottom)/2;
		pObjNode->iFrameIndex = iFrameIndex;

        pObjNode->iMatchScore = 0;
        pObjNode->fConfidence = pDetectObj[ obj_i ].fConfidence;
        pObjNode->iObjType    = pDetectObj[ obj_i ].char_class;

        pObjNode->pMatchList = NULL;

        pObjInfoList = (tObjectInfoList *)ObjectNodePop(&pTask->m_ObjInfoListFreelist);
        if (pObjInfoList)
        {
            memset(pObjInfoList,0,sizeof(tObjectInfoList));

            pObjInfoList->iObjCode = pTask->m_iCurObjCode;

            pObjNode->pMatchList = pObjInfoList;

            ObjectNodePush(pObjNode,&pObjInfoList->tInfoList);

            pObjInfoList->iInfoCount=1;

            pObjInfoList->iDetectFailCount = 0;
            pObjInfoList->iMatchScore = 0;

            pObjInfoList->iMaxObjWidth = pObjNode->tRectPos.right - pObjNode->tRectPos.left;

            printf("%s: Tracking FrameIndex:%d  new obj:%d DetectListCount:%d\n",__func__,pObjNode->iFrameIndex,pTask->m_iCurObjCode,pTask->m_ObjDetectlist.count);

            ObjectNodePush(pObjInfoList,&pTask->m_ObjDetectlist);

            pTask->m_iCurObjCode++;
            printf(" *********************%d\n",__LINE__);
        }
        else
        {
            printf(" *********************%d\n",__LINE__);
            ObjectNodePush(pObjNode,&pTask->m_ObjInfoFreelist);
            ObjectNodePush(pObjInfoList,&pTask->m_ObjInfoListFreelist);
        }
    }
}

/***************************************************************
funcName  : UpObjList
funcs	  : updata list
Param In  : pTask--task;
Prame Out :
return    :
**************************************************************/
void TrackerUpObjList(TTrackTaskMain *pTask)
{
    int i = 0;
    tObjectInfoList* pObjInfoList = NULL;
    struct listnode* node         = NULL;
    struct listnode* nextNode     = NULL;
    struct listnode* preNode      = NULL;
    tObjectInfo* tempObj = NULL;
    int iDetectThreld,iFrameIndex = 0;

    node = pTask->m_ObjDetectlist.head;

    while (node&&node->data)
    {
        nextNode = node->next;

        pObjInfoList = (tObjectInfoList*)node->data;

        tempObj = (tObjectInfo*)listnodeTail(&pObjInfoList->tInfoList);
        iDetectThreld = 6;

        if (pObjInfoList->iDetectFailCount>iDetectThreld)
        {
            printf("*********************%d\n",__LINE__);
            preNode = node->prev;

            if ( i<MAX_TRACK_NUM )
            {
                pTask->m_TrackResult[i].status  = DETECT_OBJ_STATUS_DELETE;
                pTask->m_TrackResult[i].iFrameIndex = tempObj->iFrameIndex; //tempObj->iDFrameIndex;
                pTask->m_TrackResult[i].iObjCode = pObjInfoList->iObjCode;
                pTask->m_TrackResult[i].iObjType = tempObj->iObjType;
                pTask->m_TrackResult[i].bCap = OSAL_FALSE;
                ++i;
            }

            ObjectNodeRestore(&pObjInfoList->tInfoList,&pTask->m_ObjInfoFreelist);

            memset(pObjInfoList,0,sizeof(tObjectInfoList));

            ObjectNodePush(pObjInfoList,&pTask->m_ObjInfoListFreelist);

            if (preNode)
            {
                if (nextNode)
                {
                    nextNode->prev =preNode;
                }
                preNode->next = nextNode;
            }
            else
            {
                if (nextNode)
                {
                    nextNode->prev =NULL;
                }
            }
            if (preNode==NULL)
            {
                pTask->m_ObjDetectlist.head = nextNode;
            }
            if (nextNode==NULL)
            {
                pTask->m_ObjDetectlist.tail = preNode;
            }
            pTask->m_ObjDetectlist.count--;

            listnodeFree(node);
        }
        else
        {
            printf("*********************%d\n",__LINE__);
            pObjInfoList->iDetectFailCount++;

            if(i < MAX_TRACK_NUM && pObjInfoList->iInfoCount == 1)
            {
                pTask->m_TrackResult[i].status |= DETECT_OBJ_STATUS_NEW;
            }
            if(i < MAX_TRACK_NUM && pObjInfoList->iInfoCount >= 3)
            {
                //printf("*********************%d\n",__LINE__);
                pTask->m_TrackResult[i].iObjCode = pObjInfoList->iObjCode;
//                if(pObjInfoList->iInfoCount == 3 )
//                {
//                    pTask->m_TrackResult[i].iObjCode++;
//                }
                pTask->m_TrackResult[i].bCap = OSAL_FALSE;

                pTask->m_TrackResult[i].iXStart = tempObj->tRectPos.left;
                pTask->m_TrackResult[i].iXEnd   = tempObj->tRectPos.right;
                pTask->m_TrackResult[i].iYStart = tempObj->tRectPos.top;
                pTask->m_TrackResult[i].iYEnd   = tempObj->tRectPos.bottom;
                printf("****(%d,%d)*********(%d,%d)********%d\n",tempObj->tRectPos.left,tempObj->tRectPos.top,tempObj->tRectPos.right,tempObj->tRectPos.bottom,__LINE__);

                pTask->m_TrackResult[i].iFrameIndex = tempObj->iFrameIndex;

                pTask->m_TrackResult[i].status  = DETECT_OBJ_STATUS_NORMAL;
                pTask->m_TrackResult[i].iObjType = tempObj->iObjType;
                pTask->m_TrackResult[i].fConfidence = tempObj->fConfidence;
                ++i;
            }else if(i < MAX_TRACK_NUM && pObjInfoList->iInfoCount < 3){           //middle status
                pTask->m_TrackResult[i].iObjCode = pObjInfoList->iObjCode;
                pTask->m_TrackResult[i].bCap = OSAL_FALSE;

                pTask->m_TrackResult[i].iXStart = tempObj->tRectPos.left;
                pTask->m_TrackResult[i].iXEnd   = tempObj->tRectPos.right;
                pTask->m_TrackResult[i].iYStart = tempObj->tRectPos.top;
                pTask->m_TrackResult[i].iYEnd   = tempObj->tRectPos.bottom;
                pTask->m_TrackResult[i].iFrameIndex = tempObj->iFrameIndex;

                pTask->m_TrackResult[i].status  = DETECT_OBJ_STATUS_MIDDLE;
                pTask->m_TrackResult[i].iObjType = tempObj->iObjType;
                pTask->m_TrackResult[i].fConfidence = tempObj->fConfidence;
                ++i;
            }
            if (pObjInfoList->tInfoList.count > 100)
            {//prevent list too long
                //delete first node
                tempObj = (tObjectInfo *)ObjectNodePop(&pObjInfoList->tInfoList);
                if (tempObj)
                {
                    ObjectNodePush(tempObj,&pTask->m_ObjInfoFreelist);
                }
            }
        }
        node = nextNode;
    }
    pTask->m_iResultNum = i;
    //printf("*******%d**************%d\n",i,__LINE__);
}