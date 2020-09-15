/****************************************************************************************
********
********   file name:	BaseFunction.h 
********   description: base data function
********   version:    V1.0
********   author:	   zhangsong
********   time:	   2019-11-26 10:09
********
*****************************************************************************************/


#ifndef   BASEFUNCTION_H
#define   BASEFUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#pragma once 

//#define USENCS 1
#ifndef USENCS
#define NNIE 1
#endif
//for big arr
#define USEBIGMEM  1

#include "../osal_type.h"
//#include <math.h>
#include <string.h>
#include <stdlib.h>
//#include "zstracker.h"


#define BF_ALIGNBYTE4(value) ((value+3)>>2)
#define BF_ALIGNBYTE8(value) ((value+7)>>3)
#define BF_ALIGNBYTE32(value) ((value+31)>>5)
#define IMAGEWIDTHBYTES(bits)  ((((bits) + 31) >> 5) << 2)

#define BF_TAIBIT(value,x) (((value)<<(x))>>(x))

#define BF_GRAYVALUE(B,G,R) ((R*38+G*75+B*15)>>7)

#define ROTATE_LEFT(c, N, n) ((c) = ((c)>>((N)-(n))) | ((c)<<(n)))

#define ROTATE_RIGHT(c, N, n) ((c) = ((c)<<((N)-(n))) | ((c)>>(n)))   

#define MOVEBIT_RIGHT(mk, mj, buff, N, n)\
	{\
	(mk) += (n);\
	(mj) += (n);\
	(buff) += ((mk)>>3);\
	(mk) &= ((N)-1);\
}

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef ABS
#define ABS(x)	 (((x) < 0)? -(x) : (x))
#endif


#ifdef USEBIGMEM
OSAL_VOID VD_MALLOC_BIGMEM();
OSAL_VOID VD_FREE_BIGMEM();
OSAL_INT32 GetMemMessage(OSAL_INT32 *iTotalMem,OSAL_INT32 *iEnableUseMem);
OSAL_PCHAR  inMEM_MOLLOC(OSAL_UINT32 memlen);
#endif

#ifdef USEBIGMEM
#define MEMO_FREE(p)\
{\
	p=OSAL_NULL;\
}
#define MEMO_MALLOC(p, type, size)\
{\
	p =  (type *) inMEM_MOLLOC((size)*sizeof(type));\
	while(p == OSAL_NULL)\
	{\
	p = OSAL_NULL;\
	}\
}
#else
#define MEMO_FREE(p)\
{\
	if( p )\
{\
	free(p);\
	p=OSAL_NULL;\
}\
}
#define MEMO_MALLOC(p, type, size)\
	{\
	MEMO_FREE(p);\
	p = (type *) malloc( (size)*sizeof(type));\
	while(p == OSAL_NULL)\
	{\
	p = OSAL_NULL;\
}\
}
#endif

#define MAX_RECT_NUM          512

#define  MIN_INT16(minx, sum, a, b)\
{\
	sum = (a) + (b);\
	b = (a) - (b); \
	a += b * (b >> 31);\
	minx = (sum - a);\
}

#define MAX_INT16(maxx, sum, a, b)\
{\
	sum = (a) + (b);\
	a = (a) - (b); \
	b -= a * (a >> 31);\
	maxx = (sum - b);\
}

typedef  struct TagVRECT{     
    OSAL_INT16   left;       
    OSAL_INT16   right;      
    OSAL_INT16   top;        
    OSAL_INT16   bottom;   
}VRECT,  *PVRECT;

typedef  struct  TagPosition{   
	OSAL_UINT16  x;
	OSAL_UINT16  y;	
}TPosition, *PTPosition;

typedef  struct TagVAREA{    
	VRECT       tRect;  
	OSAL_UINT32 nCount; 
	TPosition   tTop;  
	TPosition   tBottom; 
}VAREA,  *PVAREA;


typedef struct TagPoint
{
	OSAL_UINT16 x;					
	OSAL_UINT16 y;					
	OSAL_UCHAR bCurValue;			
	OSAL_UCHAR ucRes[3];           
	OSAL_UCHAR *lpCur;				
}TPoint, *PTPoint;


typedef struct tagBMPFILEHEADER {
	OSAL_UINT16  bfType;
	OSAL_UINT32  bfSize;
	OSAL_UINT16  bfReserved1;
	OSAL_UINT16  bfReserved2;
	OSAL_UINT32  bfOffBits;

} BMPFILEHEADER, *PBMPFILEHEADER;

typedef struct tagBMPINFOHEADER{
	OSAL_UINT32   biSize;
	OSAL_UINT32   biWidth;
	OSAL_UINT32   biHeight;
	OSAL_UINT16   biPlanes;
	OSAL_UINT16   biBitCount;
	OSAL_UINT32   biCompression;
	OSAL_UINT32   biSizeImage;
	OSAL_INT32    biXPelsPerMeter;
	OSAL_INT32    biYPelsPerMeter;
	OSAL_UINT32   biClrUsed;
	OSAL_UINT32   biClrImportant;
} BMPINFOHEADER, *PBMPINFOHEADER;

//image info struct
typedef struct TagImageInfo
{
    OSAL_PUCHAR  pImageData;  //point image data
    OSAL_PUCHAR  pImageUV;    //point image uv(if YUV)
    OSAL_INT32   ImageW;      //image width
    OSAL_INT32   ImageH;      //image height
    OSAL_INT32   nChannel;    //image channel
    OSAL_INT32   iImageLock;  //block flag
}TImageInfo,*PTImageInfo;

typedef struct TagFrameInfo
{
    TImageInfo    Img;         //frame data
    OSAL_INT32    nFrameIndex;   //frame ID
}TFrameInfo, *PTFrameInfo;

OSAL_VOID BF_WriteBMPFile(OSAL_PCHAR fileName, OSAL_PUCHAR lpSrc, OSAL_INT32 nw, OSAL_INT32 nh);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASEFUNCTION_H */

