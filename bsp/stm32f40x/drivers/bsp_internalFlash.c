/**
  ******************************************************************************
  * @file    bsp_internalFlash.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   内部FLASH读写测试范例
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_internalFlash.h"   
#define WRITEFLASH
#define READFLASH
/*准备写入的测试数据*/
#define DATA_32                 ((uint32_t)0x55AA)
char fpga_ver[4]={0x07,0x08,0x09,0x10};
char cpu_ver[4]={0x08,0x09,0x10,0x11};
char pcb_ver[4]={0x09,0x10,0x11,0x12};
char f210[4]={0x0a,0x0b,0x0c,0x0d};
char f543[4]={0x0b,0x0c,0x0d,0x0e};
char ver_info[20];
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/



 
static uint32_t GetSector(uint32_t Address);


/**
  * @brief  InternalFlash_Test,对内部FLASH进行读写测试
  * @param  None
  * @retval None
  */
int InternalFlash_Test(void)
{
	
	//要擦除的起始扇区(包含)及结束扇区(不包含)，如8-12，表示擦除8、9、10、11扇区
	uint32_t uwStartSector = 0;
	uint32_t uwEndSector = 0;
	
	uint32_t uwAddress = 0;
	uint32_t uwSectorCounter = 0;

	__IO uint32_t uwData32 = 0;
	__IO uint32_t uwMemoryProgramStatus = 0;
	#ifdef WRITEFLASH
  // FLASH 解锁 
  // 使能访问FLASH控制寄存器 
  FLASH_Unlock();
    
  // 擦除用户区域 (用户区域指程序本身没有使用的空间，可以自定义)
  // 清除各种FLASH的标志位
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 


	uwStartSector = GetSector(FLASH_USER_START_ADDR);
	uwEndSector = GetSector(FLASH_USER_END_ADDR);

  // 开始擦除操作 
	
  uwSectorCounter = uwStartSector;
  while (uwSectorCounter <= uwEndSector) 
  {
    // VoltageRange_3 以“字”的大小进行操作 
    if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
    { 
      //擦除出错，返回，实际应用中可加入处理 
			return -1;
    }
    // 计数器指向下一个扇区 
    if (uwSectorCounter == FLASH_Sector_11)
    {
      uwSectorCounter += 40;
    } 
    else 
    {
      uwSectorCounter += 8;
    }
  }

  /// 以“字”的大小为单位写入数据 
	/*
  uwAddress = FLASH_USER_START_ADDR;

  while (uwAddress < FLASH_USER_END_ADDR)
  {
    if (FLASH_ProgramWord(uwAddress, DATA_32) == FLASH_COMPLETE)
    {
      uwAddress = uwAddress + 4;
    }
    else
    { 
      //写入出错，返回，实际应用中可加入处理 
			return -1;
    }
  }
	*/
	
	
	/////////////write flash////////////////////
	uwAddress = FLASH_USER_START_ADDR;
	int writelen=sizeof(ver_info);
	for(int i=0;i<writelen;i++)
	{
		ver_info[i]=i+8;//user defined char
	}
	for(int i=0;i<writelen;i++)
	{
    if (FLASH_ProgramByte(uwAddress, ver_info[i]) == FLASH_COMPLETE)
    {
      uwAddress = uwAddress + 1;
    }
    else
    { 
     //write error
			return -1;
    }
  }
	
  
  FLASH_Lock(); 
#endif
	return -1;
	
#ifdef READFLASH
  /* 从FLASH中读取出数据进行校验***************************************/
  /*  MemoryProgramStatus = 0: 写入的数据正确
      MemoryProgramStatus != 0: 写入的数据错误，其值为错误的个数 */
  uwAddress = FLASH_USER_START_ADDR;
  uwMemoryProgramStatus = 0;
  
  while (uwAddress < FLASH_USER_END_ADDR)
  {
    uwData32 = *(__IO uint32_t*)uwAddress;

    if (uwData32 != DATA_32)
    {
      uwMemoryProgramStatus++;  
    }

    uwAddress = uwAddress + 4;
  }
#endif
	return -1;//added by qbc
}

int writeflash(uint32_t sector_start, uint32_t sector_end,char* buff,int len,int offset)
{
	uint32_t uwStartSector = 0;
	uint32_t uwEndSector = 0;
	
	uint32_t uwAddress = 0;
	uint32_t uwSectorCounter = 0;

	__IO uint32_t uwData32 = 0;
	__IO uint32_t uwMemoryProgramStatus = 0;
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 


	uwStartSector = GetSector(sector_start);
	uwEndSector = GetSector(sector_end);

  uwSectorCounter = uwStartSector;
	
	
	//uint32_t sectorbuff[(FLASH_USER_END_ADDR-FLASH_USER_START_ADDR)/32]={0};
//	for(int i=0,uwAddress=FLASH_USER_START_ADDR;uwAddress<FLASH_USER_END_ADDR;uwAddress+=4,i++)
//	{
//		sectorbuff[i] = *(__IO uint32_t*)uwAddress;
//	}
	
  while (uwSectorCounter <= uwEndSector) 
  {
    if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
    { 
   //erase error
			return -1;
    }
    if (uwSectorCounter == FLASH_Sector_11)
    {
      uwSectorCounter += 40;
    } 
    else 
    {
      uwSectorCounter += 8;
    }
  }
	
	/////////////write flash operations////////////////////
	uwAddress = sector_start+offset;

for(int i=0;i<len;i++)
	{
    if (FLASH_ProgramByte(uwAddress, buff[i]) == FLASH_COMPLETE)
    {
      uwAddress = uwAddress + 1;
    }
    else
    { 
     //write error
			return -1;
    }
  }  
  FLASH_Lock(); 
return 1;
}
int readflash(uint32_t sector_start,uint32_t sector_end,char * buff,int len,int offset)
{
	//uint32_t uwStartSector = 0;
	//uint32_t uwEndSector = 0;
	
	uint32_t uwAddress = 0;
	//uint32_t uwSectorCounter = 0;

	__IO uint32_t uwData32 = 0;
	__IO uint32_t uwMemoryProgramStatus = 0;
	
	uwAddress = sector_start+offset;
  uwMemoryProgramStatus = 0;
	if(len>sector_end)
	{
		return -1;
	}
	for(int i=0;i<len;i++)
	{
    uwData32 = *(__IO uint32_t*)uwAddress;
		buff[3+i]=(uwData32>>24)&0xFF;
		buff[2+i]=(uwData32>>16)&0xFF;
		buff[1+i]=(uwData32>>8)&0xFF; 
		buff[0+i]=uwData32&0xFF;
    uwAddress = uwAddress + 4;
		i=i+3;
  }
	return 0;//added by qbc
}
int write_FPGAPCB_VER(void)
{
	char fpgaverbuff[12]={0};
	for (int i=0;i<12;i++)
	{
		fpgaverbuff[i]=i+8;
	}
return	writeflash(FPGA_VER_SECTOR,FPGA_VER_SECTOR_END,fpgaverbuff,12,0);
}
/**
  * @brief  根据输入的地址给出它所在的sector
  *					例如：
						uwStartSector = GetSector(FLASH_USER_START_ADDR);
						uwEndSector = GetSector(FLASH_USER_END_ADDR);	
  * @param  Address：地址
  * @retval 地址所在的sector
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  

  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_Sector_11;  
  }

  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_Sector_12;  
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_Sector_13;  
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_Sector_14;  
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_Sector_15;  
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_Sector_16;  
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_Sector_17;  
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_Sector_18;  
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_Sector_19;  
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_Sector_20;  
  } 
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_Sector_21;  
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_Sector_22;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_Sector_23;  
  }
  return sector;
}



