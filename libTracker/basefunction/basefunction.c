/****************************************************************************************
********
********   file name:	BaseFunction.h 
********   description: base data function
********   version:    V1.0
********   author:	   zhangsong
********   time:	   2019-11-26 10:09
********
*****************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "basefunction.h"
#include <stdio.h>

#ifdef USEBIGMEM
#define  MEMORYMAXSIZE  (20 * 1024 * 1024)

static OSAL_PCHAR  g_GlobalMemory = OSAL_NULL;        //all memory
static OSAL_PCHAR  g_CurPMemory = OSAL_NULL;          //point current use memory
static OSAL_UINT32  g_nGlobal = MEMORYMAXSIZE;        //free memory size
static OSAL_CHAR g_BigMemory[MEMORYMAXSIZE];          //big arr memory
//need conside name
OSAL_VOID VD_MALLOC_BIGMEM()
{
	if (g_GlobalMemory )
    {//BigMemory has been alloc
        printf("Debug: mem has been alloc %d MB \n",MEMORYMAXSIZE/(1024*1024));
		return;
	}
    else
    {
        printf("Debug: mem new alloc %d MB \n",MEMORYMAXSIZE/(1024*1024));
    }
	g_GlobalMemory= g_BigMemory;

	if (g_GlobalMemory == OSAL_NULL)
	{
		g_nGlobal = 0;
	}
	else
	{
        g_nGlobal = MEMORYMAXSIZE;        //free memory size
		g_CurPMemory = g_GlobalMemory;
	}
}

OSAL_VOID VD_FREE_BIGMEM()
{
	g_GlobalMemory = OSAL_NULL;
	g_CurPMemory = OSAL_NULL;
	g_nGlobal = 0;
}

OSAL_INT32 GetMemMessage(OSAL_INT32 *iTotalMem,OSAL_INT32 *iEnableUseMem)
{
	if (iTotalMem == OSAL_NULL||iEnableUseMem == OSAL_NULL)
	{
		return OSAL_FAIL;
	}
	*iTotalMem = MEMORYMAXSIZE;
	*iEnableUseMem = g_nGlobal;
	return OSAL_OK;
}

OSAL_PCHAR  inMEM_MOLLOC(OSAL_UINT32 memlen)
{
	OSAL_PCHAR p=OSAL_NULL;

	//16 bit align
	memlen = (((memlen+15)>>4)<<4);

	if (g_nGlobal >= memlen)
	{
		p = g_CurPMemory;
		g_CurPMemory += memlen;
		g_nGlobal -= memlen;
	}
	return p;
}
#endif


OSAL_VOID BF_WriteBMPFile(OSAL_PCHAR fileName, OSAL_PUCHAR lpSrc, OSAL_INT32 nw, OSAL_INT32 nh)
{
	FILE *fp=NULL;
	//  file info head
	BMPFILEHEADER bfh;
	//  data buf size
	OSAL_INT32 lBufSize;
	BMPINFOHEADER bih;
	//  file handle
	fp = fopen( fileName, "wb+");
	if( !fp )
	{
		return;
	}

	lBufSize = nh *IMAGEWIDTHBYTES( 24*nw );

	//  write file info head struct
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;	
	bfh.bfType = 0x4D42; //'MB'
	bfh.bfSize = sizeof( BMPFILEHEADER ) + lBufSize + sizeof( BMPINFOHEADER );

	bfh.bfOffBits = sizeof( BMPINFOHEADER ) + sizeof( BMPFILEHEADER );

	fwrite(&bfh, sizeof(bfh), 1, fp);	
	bih.biClrImportant = 0;
	bih.biClrUsed = 0;
	bih.biCompression = 0;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;	

	bih.biSize   = sizeof(BMPINFOHEADER);
	bih.biWidth  = nw;
	bih.biHeight = nh;
	bih.biPlanes = 1;
	bih.biBitCount = 24;	

	//  write file info head
	fwrite(&bih, sizeof(bih), 1, fp);
	// write file data
	fwrite(lpSrc, lBufSize, 1, fp);
	//  close file handle
	fclose( fp ); 
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
