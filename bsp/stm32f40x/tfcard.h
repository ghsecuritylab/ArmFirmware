#include <stdlib.h>
#include "rtthread.h"
#define SECEND 0xFFFFFFFF
struct TestIndexListStc
{
	rt_uint16_t ti;
	rt_uint8_t del;
	rt_uint64_t time;
	char TestLocA[20];
	char TestLocB[20];
	char TestLocC[20];
	char unit[10];
	float a1;//raw->mA
	float b1;
	float a2;//mA->unit
	float b2;
	
	long pos;
	struct TestIndexListStc* next;
	struct TestIndexListStc* pre;	
}TestIndexList;
struct TestIndexListStc TestIndexListBegin;
struct TestIndexListStc TestIndexListEnd;
union float2byte
{
	float f;
	rt_uint8_t b[4];
}f2b;

struct rt_mutex tfio_mutex;
//open TestIndex.bin file, return testindex, return -1 if error
int AddTestIndex(struct TestIndexListStc til);
//add a value into a testindex, return -1 on error
int AddValue(int TestIndex,long time, unsigned int value);
//end a testindex
int EndTestIndex(int TestIndex);
//mark 0 on del colum in TestIndex.bin
int DeleteTestIndex(int TestIndex);
//get index array, return index count, return -1 on error
int ReadAllTestIndex();

