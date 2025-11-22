#include "relay.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	5V继电器c文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.18
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥

**********************BEGIN***********************/



void RELAY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RELAY_CLK_A, ENABLE ); //配置时钟
	
	GPIO_InitStructure.GPIO_Pin = RELAY_GPIO_PIN_FS |RELAY_GPIO_PIN_SB;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(RELAY_GPIO_PROT_A,&GPIO_InitStructure);

	
	RCC_APB2PeriphClockCmd(RELAY_CLK_B, ENABLE ); //配置时钟
	
	GPIO_InitStructure.GPIO_Pin = RELAY_GPIO_PIN_HOT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(RELAY_GPIO_PROT_B,&GPIO_InitStructure);
	
	RELAY_FS_OFF;
	RELAY_SB_OFF;
	RELAY_HOT_OFF;
}
