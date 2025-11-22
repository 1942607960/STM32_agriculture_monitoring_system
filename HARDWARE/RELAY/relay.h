#ifndef __RELAY_H
#define	__RELAY_H
#include "stm32f10x.h"

#include "sys.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	5V继电器h文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.18
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥

**********************BEGIN***********************/


/***************根据自己需求更改****************/
// 继电器 GPIO宏定义

#define	RELAY_CLK_A					RCC_APB2Periph_GPIOA
#define	RELAY_CLK_B					RCC_APB2Periph_GPIOB

#define RELAY_GPIO_PROT_A 			GPIOA
#define RELAY_GPIO_PROT_B 			GPIOB

#define RELAY_GPIO_PIN_FS 				GPIO_Pin_11
#define RELAY_GPIO_PIN_SB				GPIO_Pin_12
#define RELAY_GPIO_PIN_HOT 				GPIO_Pin_7


#define RELAY_FS_ON 	GPIO_SetBits(RELAY_GPIO_PROT_A,RELAY_GPIO_PIN_FS)	
#define RELAY_FS_OFF 	GPIO_ResetBits(RELAY_GPIO_PROT_A,RELAY_GPIO_PIN_FS)

#define RELAY_SB_ON 	GPIO_SetBits(RELAY_GPIO_PROT_A,RELAY_GPIO_PIN_SB)	
#define RELAY_SB_OFF 	GPIO_ResetBits(RELAY_GPIO_PROT_A,RELAY_GPIO_PIN_SB)	

#define RELAY_HOT_ON 	GPIO_SetBits(RELAY_GPIO_PROT_B,RELAY_GPIO_PIN_HOT)	
#define RELAY_HOT_OFF 	GPIO_ResetBits(RELAY_GPIO_PROT_B,RELAY_GPIO_PIN_HOT)	
/*********************END**********************/

void RELAY_Init(void);

#endif



