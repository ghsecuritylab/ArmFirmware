#include <stdio.h>
#include <stdlib.h>

typedef struct TestIndexStruct {
    int val;
    struct TestIndexStuct *pre;
    struct TestIndexStruct * next;
} TestIndex,TestIndexBegin;

int main() {
    struct TestIndexStruct *ti=NULL;
    int pos=0;
	do
    {
        if(pos==0)
        {
        	ti=&TestIndexBegin;    
        }
        else
        {
            struct TestIndex *pti=NULL;
            pti=malloc(sizeof(TestIndex));
            ti->next=pti;//c1
			pti=ti;//c2
			ti=ti->next;//c3
			ti->pre=pti;//c4
			ti->next=NULL;//c5
        }
        ti->val=pos+10;
    }while(pos<10);
}