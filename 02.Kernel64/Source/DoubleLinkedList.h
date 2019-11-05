#ifndef _DOUBLELINKEDLIST_H

#define _DOUBLELINKEDLIST_H
typedef struct _NODE { 
    char* m_Data;

    struct _NODE *m_spPrev;
    struct _NODE *m_spNext;

}SNode;

typedef struct _DOUBLELINKEDLIST{

    SNode *m_spHead;
    SNode *m_spTail;

    SNode *m_spIterator;

}DoubleLinkedList;

SNode* CreateNode(SNode* sNode, char *pData);

void Push_Back(DoubleLinkedList *spDLL, char *pData, SNode* node);

void Push_Empty(DoubleLinkedList *spDLL, char *pData, SNode* node);

void InitIterator(DoubleLinkedList *spDLL);

DoubleLinkedList* CreateLinkedList(DoubleLinkedList* dll);

char* IterateNextList(DoubleLinkedList *spDLL, int i);

char* IteratePrevList(DoubleLinkedList *spDLL, int i);

void Pop_Front(DoubleLinkedList *spDLL);


#endif
