#include "DoubleLinkedList.h"
#include "Console.h"
#include <stdlib.h>


SNode* CreateNode(SNode* sNode, char *pData){

	SNode* spNode = sNode;

	spNode->m_Data = pData;

	spNode->m_spPrev = 0;

	spNode->m_spNext = 0;

	return spNode;

}

void Push_Back(DoubleLinkedList *spDLL, char *pData, SNode* node){

	SNode *spNewNode = CreateNode(node, pData);

	spDLL -> m_spTail -> m_spNext = spNewNode;
	spNewNode->m_spPrev = spDLL -> m_spTail;
	
	spDLL -> m_spTail = spNewNode;
	InitIterator(spDLL);
}

void Push_Empty(DoubleLinkedList *spDLL, char *pData, SNode* node) {

	SNode *spNewNode = CreateNode(node, pData);

	spDLL->m_spHead = spNewNode;
	spDLL->m_spTail = spNewNode;

	InitIterator(spDLL);
}

void InitIterator(DoubleLinkedList *spDLL){

	spDLL -> m_spIterator = spDLL -> m_spTail;

}

DoubleLinkedList* CreateLinkedList(DoubleLinkedList* dll){

	DoubleLinkedList* spNewList = dll;

	spNewList -> m_spHead = 0;

	spNewList -> m_spTail = 0;

	InitIterator(spNewList);

	return spNewList;

}

char* IterateNextList(DoubleLinkedList *spDLL, int i){

	char* pSaveData = "\0";
	if((spDLL -> m_spIterator) != 0){

		if(spDLL -> m_spIterator != spDLL -> m_spTail){
			spDLL -> m_spIterator = spDLL -> m_spIterator ->m_spNext;
			pSaveData = spDLL-> m_spIterator ->m_Data;
		}
		else if(i == 0){
			InitIterator(spDLL);
			pSaveData = spDLL->m_spIterator->m_Data;	
		}
	}

	else
		return 0;

	return pSaveData;

}

char* IteratePrevList(DoubleLinkedList *spDLL, int i){

	char* pSaveData = "\0";
	if(i == 0){
		InitIterator(spDLL);
		pSaveData = spDLL->m_spIterator->m_Data;
	}
	else if(i < 10){
		if(spDLL -> m_spIterator !=0 ){
			if(spDLL -> m_spIterator != spDLL -> m_spHead)
			{
				spDLL -> m_spIterator = spDLL -> m_spIterator -> m_spPrev;
				pSaveData = spDLL->m_spIterator -> m_Data;
			}
		}
	}

	else
		return 0;

	return pSaveData;

}

void Pop_Front(DoubleLinkedList *spDLL){

	SNode *pIterator = spDLL -> m_spHead ->m_spNext;

	spDLL -> m_spHead = pIterator;
	InitIterator(spDLL);
}

