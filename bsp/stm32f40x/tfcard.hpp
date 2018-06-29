#include <stdlib.h>
struct TestIndexListStc
{
	int ti;
	const char* TestLoc;
	long time;
	float a;
	float b;
	const char* unit;
	
	long pos;
	struct TestIndexList* next;
	struct TestIndexList* pre;	
}TestIndexList;
//open TestIndex.bin file, return testindex, return -1 if error
int AddTestIndex(const char* TestLoc,long time,float a, float b, const char* unit);
//add a value into a testindex, return -1 on error
int AddValue(int TestIndex,long time, unsigned int value);
//end a testindex
int EndTestIndex(int TestIndex);
//mark 0 on del colum in TestIndex.bin
int DeleteTestIndex(int TestIndex);
//get index array, return index count, return -1 on error
int ReadAllTestIndex(struct TestIndexListStc ti);

