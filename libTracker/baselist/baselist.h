/****************************************************************************************
********
********   文件名:   BaseList.h 
********   功能描述: 基础链表
********   版本:     V1.0
********   作者:     zhangsong
********   日期:     2019-8-11 12:37
********  
*****************************************************************************************/


#ifndef   BaseList_H
#define   BaseList_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdio.h>

typedef struct listnode 
{
	struct listnode *next;
	struct listnode *prev;
	void *data;
}tListNode;

typedef struct List 
{
	struct listnode *head;
	struct listnode *tail;
	unsigned int count;	
}tList,*ptList;

struct listnode *listnodeAdd (struct List *, void *); //创建新的节点填充数据并加入链表尾部

void listnodeDelete (struct List *, void *); //删除匹配数据的节点

void *listnodeHead (struct List *);

void *listnodeTail (struct List *);

struct List *listNew();

void listFree (struct List *);

void listnodeFree (struct listnode *node);

void listDelete (struct List *);

void listDeleteAllNode (struct List *);

void listDeleteNode (struct List*, struct listnode *);

void* listPop(struct List*list);

/* List iteration macro. */
#define LIST_LOOP(L,V,N) \
	for ((V) = NULL,(N) = (L)->head; (N); (N) = (N)->next) \
	if (((V) = (N)->data) != NULL)
/* List iteration macro. */
#define LIST_LOOP_REVERSE(L,V,N) \
    for ((V) = NULL,(N) = (L)->tail; (N); (N) = (N)->prev) \
    if (((V) = (N)->data) != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BaseList_H */
