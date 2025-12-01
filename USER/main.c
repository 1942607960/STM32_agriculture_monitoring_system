#include "led.h"
#include "sys.h"
#include "usart.h"	 
#include "math.h"			
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "timer.h"
#include "stm32f10x_tim.h"
#include "BC26.h"	 
#include <stm32f10x.h>
#include "dht11.h"
#include "relay.h"
#include "key.h"
#include "oled.h"
#include "adcx.h"
#include "LDR.h"
#include "TS.h"
#include "FreeRTOS.h"
#include "SysTick.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

u16 DHT11_Read_6s_num=0;   //间隔6s读取一次DHT11传感器，手册上说间隔大于5s为准确的值
u8  DHT11_Data_Read_flag=0;//DHT11读取标志位


// 定义传感器数据结构体
typedef struct {
    u8 temp;           // 温度
    u8 humi;           // 湿度
    u16 light;         // 光照强度
    u16 soil; // 土壤湿度
} SensorData_t;

// 创建队列
QueueHandle_t xSensorQueue = NULL;


#define  QUEUE_LEN    5   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   sizeof(SensorData_t)   /* 队列中每个消息大小（字节） */
#define SIZE 128


//湿度阈值设定
#define HIGH_HUMI  18
#define LOW_HUMI   12
//温度阈值设定
#define HIGH_TEMP  26
#define LOW_TEMP   25
//光照阈值设定
#define HIGH_LIGHT  160
#define LOW_LIGHT  110
//土壤湿度阈值设定
#define HIGH_SOIL  6
#define LOW_SOIL  5

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		SIZE  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define SENSOR_TASK_PRIO		2
//任务堆栈大小	
#define SENSOR_STK_SIZE 		SIZE  
//任务句柄
TaskHandle_t SensorTask_Handler;
//任务函数
void sensor_task(void *pvParameters);

//任务优先级
#define PROCESS_TASK_PRIO		3
//任务堆栈大小	
#define PROCESS_STK_SIZE 		SIZE  
//任务句柄
TaskHandle_t ProcessTask_Handler;
//任务函数
void process_task(void *pvParameters);

//任务优先级
#define ACTION_TASK_PRIO		5
//任务堆栈大小	
#define ACTION_STK_SIZE 		SIZE  
//任务句柄
TaskHandle_t ActionTask_Handler;
//任务函数
void action_task(void *pvParameters);


//任务优先级
#define CLOUDCMD_TASK_PRIO		4
//任务堆栈大小	
#define CLOUDCMD_STK_SIZE 		SIZE  
//任务句柄
TaskHandle_t CloudcmdTask_Handler;
//任务函数
void cloudcmd_task(void *pvParameters);



EventGroupHandle_t Event_Handle = NULL;


#define HIGH_HUMI_EVENT  (0x01 << 0)//设置事件掩码的位0  左移0位
#define LOW_HUMI_EVENT  (0x01 << 1)//设置事件掩码的位1  左移1位
#define HIGH_TEMP_EVENT  (0x01 << 2)//设置事件掩码的位2  左移2位
#define LOW_TEMP_EVENT  (0x01 << 3)//设置事件掩码的位3  左移3位
#define HIGH_SOIL_EVENT  (0x01 << 4)//设置事件掩码的位4  左移4位
#define LOW_SOIL_EVENT  (0x01 << 5)//设置事件掩码的位5  左移5位
#define HIGH_LIGHT_EVENT  (0x01 << 6)//设置事件掩码的位6  左移6位
#define LOW_LIGHT_EVENT  (0x01 << 7)//设置事件掩码的位7  左移7位

 int main(void)
 {	
	SystemInit();
    SysTick_Init(72);	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4，全给抢占优先级
	TIM4_Int_Init(9999,71);
	
	DHT11_Init();
	LDR_Init();
	TS_Init();
	ADCx_Init(); //ADC的初始化
	RELAY_Init();
	LED_Init(); 
    uart_init(115200);//串口1初始化，可连接PC进行打印模块返回数据
	Uart1_SendStr("UART1 Init Successful\r\n");
    uart2_init(115200);//初始化和BC20连接串口	
	Uart2_SendStr("UART2 Init Successful\r\n");
	uart3_init(115200);
	Uart3_SendStr("UART3 Init Successful\r\n");
    //IWDG_Init(7,625);    //8S一次
    
	
	OLED_Init();
	printf("Start \n");
	delay_ms(1000);
	
	OLED_Clear();
//显示“光照强度:”
	OLED_ShowChinese(0,0,0,16,1);
	OLED_ShowChinese(16,0,1,16,1);
	OLED_ShowChinese(32,0,2,16,1);
	OLED_ShowChinese(48,0,3,16,1);
	OLED_ShowChar(64,0,':',16,1);
	
	//显示“土壤湿度:”
	OLED_ShowChinese(0,18,4,16,1);
	OLED_ShowChinese(16,18,5,16,1);
	OLED_ShowChinese(32,18,6,16,1);
	OLED_ShowChinese(48,18,7,16,1);
	OLED_ShowChar(64,18,':',16,1);
	
	OLED_ShowChinese(0,36,8,16,1);
	OLED_ShowChinese(16,36,9,16,1);
	OLED_ShowChinese(32,36,10,16,1);
	OLED_ShowChinese(48,36,6,16,1);
	OLED_ShowChinese(64,36,7,16,1);
	OLED_ShowChar(80,36,':',16,1);
	
	while(BC26_Init()); 		//单片机和模块的链接
    BC26_PDPACT();				//看下是否激活
    BC26_RegALIYUNIOT();		//注册到电信云华为云平台
		
//	Key_Init();	

	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    
	vTaskStartScheduler();          //开启任务调度
 }
 
//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
    xSensorQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */  
	

	
	if (xSensorQueue == NULL ) {
        printf("队列创建失败！请检查堆内存配置。\r\n");
    }
	
	// 2. 创建事件组（关键！补充这部分）
    Event_Handle = xEventGroupCreate();
    if (Event_Handle == NULL) {
        printf("事件组创建失败！堆内存不足或配置错误\r\n");
        // 可选：创建失败时的兜底处理（如卡死提示、复位）
        while(1) {
            printf("Event Group Create Error!\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
	
	
	//创建 传感器采集任务
    xTaskCreate((TaskFunction_t )sensor_task,     
                (const char*    )"sensor_task",   
                (uint16_t       )SENSOR_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )SENSOR_TASK_PRIO,
                (TaskHandle_t*  )&SensorTask_Handler); 
	//创建 处理任务
    xTaskCreate((TaskFunction_t )process_task,     
                (const char*    )"process_task",   
                (uint16_t       )PROCESS_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )PROCESS_TASK_PRIO,
                (TaskHandle_t*  )&ProcessTask_Handler);
	//创建 执行任务
    xTaskCreate((TaskFunction_t )action_task,     
                (const char*    )"action_task",   
                (uint16_t       )ACTION_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )ACTION_TASK_PRIO,
                (TaskHandle_t*  )&ActionTask_Handler);


	//创建 云端指令任务
    xTaskCreate((TaskFunction_t )cloudcmd_task,     
                (const char*    )"cloudcmd_task",   
                (uint16_t       )CLOUDCMD_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )CLOUDCMD_TASK_PRIO,
                (TaskHandle_t*  )&CloudcmdTask_Handler);			
	
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
} 

//执行任务(5)
void action_task(void *pvParameters) {
	
	EventBits_t r_event;  /* 定义一个事件接收变量 */	
	
	while (1) {
		r_event = xEventGroupWaitBits(Event_Handle,  /* 事件对象句柄 */
                                  HIGH_HUMI_EVENT|LOW_HUMI_EVENT|HIGH_TEMP_EVENT|LOW_TEMP_EVENT|HIGH_SOIL_EVENT|LOW_SOIL_EVENT|HIGH_LIGHT_EVENT|LOW_LIGHT_EVENT,/* 接收线程感兴趣的事件参数 HIGH_EVENT|LOW_EVENT：表示要一直等到位0和位1都被设置，没设置就阻塞 */
                                  pdTRUE,   /* 退出时清除事件位 */
                                  pdFALSE,   /* 满足感兴趣的所有事件 */
                                  portMAX_DELAY);/* 指定超时事件,一直等 */
		
		if(r_event & HIGH_HUMI_EVENT)
		{	
			RELAY_FS_ON;
			
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"FS\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("湿度过高，风扇开启\r\n");
		}
		else if(r_event & LOW_HUMI_EVENT)
		{
			RELAY_FS_OFF;

			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"FS\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("湿度过低，风扇关闭\r\n");
		}
        else if(r_event & HIGH_TEMP_EVENT)
		{

			RELAY_HOT_OFF;
			
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"HOT\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("温度过高，加热片关闭\r\n");
		}
		else if(r_event & LOW_TEMP_EVENT)
		{
	
			RELAY_HOT_ON;
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"HOT\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("温度过低，加热片开启\r\n");
		}
		else if(r_event & HIGH_SOIL_EVENT)
		{
			
			RELAY_SB_OFF;
			
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"SB\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("土壤湿度过高，水泵关闭\r\n");
		}
		else if(r_event & LOW_SOIL_EVENT)
		{
			
			RELAY_SB_ON;
			
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"SB\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("土壤湿度过低，水泵打开\r\n");
		}	
        else if(r_event & HIGH_LIGHT_EVENT)
		{
			GPIO_ResetBits(LED2_PORT,LED2_PIN);
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"LIGHT\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("光照充足，补光灯不开启\r\n");
		}
		else if(r_event & LOW_LIGHT_EVENT)
		{
			GPIO_SetBits(LED2_PORT,LED2_PIN);
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"LIGHT\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray);//上报风扇状态
			printf("光照过低，补光灯开启\r\n");
		}
		}
    }



//处理任务(4)
void process_task(void *pvParameters) {
    
	SensorData_t xReceivedData;
	
    while (1) {
				
        // 从传感器队列接收数据
        if (xQueueReceive(xSensorQueue, &xReceivedData, portMAX_DELAY) == pdTRUE) {            
			printf("[传感器采集队列]的数据已接收\r\n");
			
			// 湿度阈值判断
            if (xReceivedData.humi > HIGH_HUMI ) {
		
				xEventGroupSetBits(Event_Handle,HIGH_HUMI_EVENT);	
                printf("湿度过高r\n");
					
            }
			else if( xReceivedData.humi < LOW_HUMI){
				
				xEventGroupSetBits(Event_Handle,LOW_HUMI_EVENT);
				printf("湿度过低\r\n");
				
			}
			
			// 温度阈值判断
            if (xReceivedData.temp > HIGH_TEMP ) {
		
				xEventGroupSetBits(Event_Handle,HIGH_TEMP_EVENT);	
                printf("温度过高r\n");
					
            }
			else if( xReceivedData.temp < HIGH_TEMP){
				
				xEventGroupSetBits(Event_Handle,LOW_TEMP_EVENT);
				printf("温度过低\r\n");
				
			}
			
			// 土壤湿度阈值判断
            if (xReceivedData.soil > HIGH_SOIL ) {
		
				xEventGroupSetBits(Event_Handle,HIGH_SOIL_EVENT);	
                printf("土壤湿度过高r\n");
					
            }
			else if( xReceivedData.soil < LOW_SOIL){
				
				xEventGroupSetBits(Event_Handle,LOW_SOIL_EVENT);
				printf("土壤湿度过低\r\n");
				
			}
			// 光照阈值判断
            if (xReceivedData.light > HIGH_LIGHT ) {
		
				xEventGroupSetBits(Event_Handle,HIGH_LIGHT_EVENT);	
                printf("光照充足r\n");
					
            }
			else if( xReceivedData.light < LOW_LIGHT){
				
				xEventGroupSetBits(Event_Handle,LOW_LIGHT_EVENT);
				printf("光照不足\r\n");
				
			}
			
			
			
			
			
			// 通过串口打印数据
            printf("温度: %d, 湿度: %d, 光照: %d, 土壤湿度: %d\r\n",
                   xReceivedData.temp,
                   xReceivedData.humi,
                   xReceivedData.light,
                   xReceivedData.soil);
			
			OLED_ShowNum(68,0,xReceivedData.light,3,16,1);
			OLED_ShowNum(68,18,xReceivedData.soil,2,16,1);
			OLED_ShowNum(84,36,xReceivedData.temp,2,16,1);
			OLED_ShowNum(106,36,xReceivedData.humi,2,16,1);
			

			BC26_HUAWEIYUNSendData(xReceivedData.temp, xReceivedData.humi, xReceivedData.light, xReceivedData.soil);
			printf("数据上传成功!\r\n");
			
			
        }else{
			printf("[WARN][传感器采集队列]的数据接收失败！\r\n");
		}
    }
}

//传感器采集任务(2)
void sensor_task(void *pvParameters){
    
	SensorData_t xSensorData;
	
    while (1) {
        // 读取传感器数据
        xSensorData.light = LDR_LuxData();   // 光照传感器
        xSensorData.soil = TS_GetData();   // 土壤湿度传感器
        DHT11_Read_Data(&xSensorData.temp, &xSensorData.humi); // 温湿度传感器

        // 发送到队列（阻塞直到队列有空位）
        if (xQueueSend(xSensorQueue, &xSensorData, portMAX_DELAY) == pdPASS) {
			printf("[传感器采集队列]的数据已发送\r\n");
        }else{
			 printf("[传感器采集队列]的数据发送失败\r\n");
		}

        // 控制采集频率（如每秒1次）
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


// 云端指令任务(5)
void cloudcmd_task(void *pvParameters) {
	while (1) {	
		
		BC26_RECData();
		vTaskDelay(pdMS_TO_TICKS(500));
	
	}
}
//我是主分支
//对主分支做更改