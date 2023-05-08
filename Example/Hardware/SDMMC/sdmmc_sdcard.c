#include "sdmmc_sdcard.h"
#include "string.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//SD卡驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

SD_HandleTypeDef        SDCARD_Handler;             //SD卡句柄   
HAL_SD_CardInfoTypeDef  SDCardInfo;					//SD卡信息结构体
DMA_HandleTypeDef SDTxDMAHandler,SDRxDMAHandler;    //SD卡DMA发送和接收句柄

//如果使用DMA的话下面两个变量用来标记SD卡读写是否完成
static volatile u8 SDCardWriteStatus=0,SDCardReadStatus=0; 

//SD_ReadDisk/SD_WriteDisk函数专用buf,当这两个函数的数据缓存区地址不是4字节对齐的时候,
//需要用到该数组,确保数据缓存区地址是4字节对齐的.
__align(4) u8 SDIO_DATA_BUFFER[512];

//SD卡初始化,SDMMC频率为200MHz，SD卡最大频率25MHz
//返回值:0 初始化正确；其他值，初始化错误
u8 SD_Init(void)
{
    u8 SD_Error;
    
    //初始化时的时钟不能大于400KHZ 
    SDCARD_Handler.Instance=SDMMC1;
    SDCARD_Handler.Init.ClockEdge=SDMMC_CLOCK_EDGE_RISING;              //上升沿     
    SDCARD_Handler.Init.ClockPowerSave=SDMMC_CLOCK_POWER_SAVE_DISABLE;  //空闲时不关闭时钟电源
    SDCARD_Handler.Init.BusWide=SDMMC_BUS_WIDE_4B;                      //4位数据线
    SDCARD_Handler.Init.HardwareFlowControl=SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;//关闭硬件流控
    SDCARD_Handler.Init.ClockDiv=SDMMC_NSpeed_CLK_DIV;               	//SD传输时钟频率最大25MHZ
    
    SD_Error=HAL_SD_Init(&SDCARD_Handler);
    if(SD_Error!=HAL_OK) return 1;
	
	//获取SD卡信息
	HAL_SD_GetCardInfo(&SDCARD_Handler,&SDCardInfo);
    return 0;
}

//SDMMC底层驱动，时钟使能，引脚配置，DMA配置
//此函数会被HAL_SD_Init()调用
//hsd:SD卡句柄
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_SDMMC1_CLK_ENABLE();  //使能SDMMC1时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();   //使能GPIOC时钟
    __HAL_RCC_GPIOD_CLK_ENABLE();   //使能GPIOD时钟
    
    //PC8,9,10,11,12
    GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;      //推挽复用
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     //高速
    GPIO_Initure.Alternate=GPIO_AF12_SDIO1; //复用为SDIO
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);     //初始化
    
    //PD2
    GPIO_Initure.Pin=GPIO_PIN_2;            
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);     //初始化

#if (SD_DMA_MODE==1)                        //使用DMA模式
    HAL_NVIC_SetPriority(SDMMC1_IRQn,2,0);  //配置SDMMC1中断，抢占优先级2，子优先级0
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);        //使能SDMMC1中断
#endif
}

//得到卡信息
//cardinfo:卡信息存储区
//返回值:错误状态
u8 SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo)
{
    u8 sta;
	sta=HAL_SD_GetCardInfo(&SDCARD_Handler,cardinfo);
    return sta;
}

//判断SD卡是否可以传输(读写)数据
//返回值:SD_TRANSFER_OK 传输完成，可以继续下一次传输
//		 SD_TRANSFER_BUSY SD卡正忙，不可以进行下一次传输
u8 SD_GetCardState(void)
{
  return((HAL_SD_GetCardState(&SDCARD_Handler)==HAL_SD_CARD_TRANSFER )?SD_TRANSFER_OK:SD_TRANSFER_BUSY);
}

#if (SD_DMA_MODE==1)        //DMA模式

//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_ReadDisk(u8* buf,u32 sector,u32 cnt)
{
    u8 sta=HAL_ERROR;
	SDCardReadStatus=0;
	
	if(HAL_SD_ReadBlocks_DMA(&SDCARD_Handler,(uint8_t*)buf,(uint32_t)sector,(uint32_t)cnt)==HAL_OK)
	{
		while(SDCardReadStatus==0){};	//等待读完成
		
		SDCardReadStatus=0;
		while(SD_GetCardState()){};		//等待SD卡空闲
		sta=HAL_OK;
	}
	
    return sta;
}  

//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;	
u8 SD_WriteDisk(u8 *buf,u32 sector,u32 cnt)
{   
    u8 sta=HAL_ERROR;
	
	SDCardWriteStatus=0;
	
	if(HAL_SD_WriteBlocks_DMA(&SDCARD_Handler,(uint8_t*)buf,(uint32_t)sector,(uint32_t)cnt)==HAL_OK)
	{
		while(SDCardWriteStatus==0){};	//等待写完成
		
		SDCardWriteStatus=0;
		while(SD_GetCardState()){};		//等待SD卡空闲
		sta=HAL_OK;
	}
		
    return sta;
} 

//SDMMC1中断服务函数
void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&SDCARD_Handler);
}

//SDMMC1写完成回调函数
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
	SDCardWriteStatus=1; //标记写完成
}

//SDMMC1读完成回调函数
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
	SDCardReadStatus=1;	//标记读完成
}

#else                                   //轮训模式

//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_ReadDisk(u8* buf,u32 sector,u32 cnt)
{
    u8 sta=HAL_OK;
	u32 timeout=SD_TIMEOUT;
    long long lsector=sector;
    INTX_DISABLE();//关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
	sta=HAL_SD_ReadBlocks(&SDCARD_Handler, (uint8_t*)buf,lsector,cnt,SD_TIMEOUT);//多个sector的读操作
	
	//等待SD卡读完
	while(SD_GetCardState()!=SD_TRANSFER_OK)
    {
		if(timeout-- == 0)
		{	
			sta=SD_TRANSFER_BUSY;
            break;
		}
    }
    INTX_ENABLE();//开启总中断
    return sta;
}  


//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;	
u8 SD_WriteDisk(u8 *buf,u32 sector,u32 cnt)
{   
    u8 sta=HAL_OK;
	u32 timeout=SD_TIMEOUT;
    long long lsector=sector;
    INTX_DISABLE();//关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
	sta=HAL_SD_WriteBlocks(&SDCARD_Handler,(uint8_t*)buf,lsector,cnt,SD_TIMEOUT);//多个sector的写操作
		
	//等待SD卡写完
	while(SD_GetCardState()!=SD_TRANSFER_OK)
    {
		if(timeout-- == 0)
		{	
			sta=SD_TRANSFER_BUSY;
            break;
		}
    }    
	INTX_ENABLE();//开启总中断
    return sta;
}
#endif




