#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

void LED_Init(void);//初始化
void BC26CTR_Init(void);


/*  LED时钟端口、引脚定义 */
#define LED1_PORT 			GPIOC  
#define LED1_PIN 			GPIO_Pin_13
#define LED1_PORT_RCC		RCC_APB2Periph_GPIOC

#define LED2_PORT 			GPIOA   
#define LED2_PIN 			GPIO_Pin_8
#define LED2_PORT_RCC		RCC_APB2Periph_GPIOA





#define LED1 PCout(13)	
#define LED2 PAout(8)		

#define PWRKEY  PCout(7)
//#define RESET   PAout(8)

#endif
