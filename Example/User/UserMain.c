#include "UserMain.h"
#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h"
#include "ltdc.h"
#include "lcd.h"
#include "sdram.h"
#include "usmart.h"
#include "mpu.h"
#include "malloc.h"
#include "usb_device.h"
/*********************************************************************************/
/*
 * 配置参数初始化
 */
void UserConfigInit() {

}

/*
 * 系统初始化
 */
void UserSysInit() {
	 //Cache_Enable();
	//MPU_Memory_Protection();
	 //HAL_PWREx_EnableUSBVoltageDetector();
		//Stm32_Clock_Init(192,5,2,4);  		    //????,400Mhz 
	delay_init(400);
	uart_init(115200);
	//printf("666");
  LED_Init();
	SDRAM_Init();
	LCD_Init();		

 	my_mem_init(SRAMIN);
	my_mem_init(SRAMEX);
	my_mem_init(SRAMDTCM);
  POINT_COLOR=RED;                                                                                                                          
	LCD_ShowString(30,50,200,16,16,"Apollo STM32H7"); 
	LCD_ShowString(30,70,200,16,16,"USB Virtual USART TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2018/7/19");	 
 	LCD_ShowString(30,130,200,16,16,"USB Connecting...");//提示USB开始连接
	MX_USB_DEVICE_Init();

}

/*
 * 主循环,任务分发
 */
void UserLoop() {
//LCD_Clear(RED);
//	delay_ms(500);
//	LCD_Clear(WHITE);
//	delay_ms(500);
//	LCD_Clear(BLUE);
//	delay_ms(500);
}
void UserErrorHandler(){
		printf("system error!\n");
}
