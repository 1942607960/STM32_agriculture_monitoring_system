#ifndef __BC26_H
#define __BC26_H	
#include "usart.h"
#include <stm32f10x.h>
#include "usart.h"
#include "SysTick.h"

////////////////////////修改要素/////////////////////////////////
#define  proid     "67e6a76711e2161f155b9f27"      //产品ID 好像没用

#define  devid     "67e6a76711e2161f155b9f27_86429"//设备ID                      								
#define  clientId  "67e6a76711e2161f155b9f27_86429_0_0_2025032903" //客户端ID  										
#define  username  "67e6a76711e2161f155b9f27_86429"         //用户名
#define  passwd    "db40592f2438ed4c3821ce68f2c868b767c6a09707ce9bffb6de5d6d318a8585" 	//密码
#define  ServerIP  "c9b1dbfa16.iotda-device.cn-south-4.myhuaweicloud.com" //域名IP
//还有一个服务ID 直接写在当页了
///////////////////////////////////////////////////////////////////////

void Clear_Buffer(void);//清空缓存	
int  BC26_Init(void);
void BC26_PDPACT(void);
void BC26_RECData(void);
void BC26_RegALIYUNIOT(void);
void BC26_ALYIOTSenddata(u8 *len,u8 *data);
void BC26_ALYIOTSenddata1(u8 *len,u8 *data);
void BC26_HUAWEIYUNSendData(uint8_t temp,uint8_t humi,uint16_t light,uint16_t value);

typedef struct
{
   uint8_t CSQ;    
   uint8_t Socketnum;   //编号
   uint8_t reclen[10];   //获取到数据的长度
   uint8_t res;      
   uint8_t recdatalen[10];
   uint8_t recdata[100];
} BC26;

#endif


