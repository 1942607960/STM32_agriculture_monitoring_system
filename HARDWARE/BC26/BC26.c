#include "BC26.h"
#include "string.h"
//AT+NSOCL=0
#include "led.h"
#include "relay.h"
char *strx,*extstrx;
char atstr[BUFLEN];
int err;    //全局变量
char atbuf[BUFLEN];
char objtnum[BUFLEN];//观察号
char distnum[BUFLEN];//观察号

int socketnum=0;//当前的socket号码
BC26 BC26_Status;


void Clear_Buffer(void)//清空串口2缓存
{
    printf(buf_uart2.buf);  //清空前打印信息
    delay_ms(300);
    buf_uart2.index=0;
    memset(buf_uart2.buf,0,BUFLEN);//void *memset(void *s, int c, size_t n); s:要填充的内存块的指针 c:要设置的值 n:要填充的字节数
}

int BC26_Init(void)
{
    int errcount = 0;
    err = 0;    //判断模块卡是否就绪最重要
    printf("start init BC26\r\n");
	
	Clear_Buffer();	
    Uart2_SendStr("ATi\r\n");
    delay_ms(3000);
    printf(buf_uart2.buf);      //打印收到的串口信息
    printf("get back BC26\r\n");
    strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返回OK const是原函数参数的要求
    while(strx==NULL)
    {
        printf("\r\n单片机正在连接到模块...\r\n");
        Clear_Buffer();	
        Uart2_SendStr("ATi\r\n");
        delay_ms(300);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返回OK
    }
	
	Uart2_SendStr("AT+QCCID\r\n");//查询SIM卡的QCCID
    delay_ms(300);
	
    Clear_Buffer();	
    Uart2_SendStr("AT+CIMI\r\n");//获取IMSI 国际移动用户识别码
    strx=strstr((const char*)buf_uart2.buf,(const char*)"ERROR");//只要卡不错误 基本就成功
    if(strx==NULL)
    {
        printf("我的卡号是 : %s \r\n",&buf_uart2.buf[8]);
        Clear_Buffer();	
        delay_ms(300);
    }
    else
    {
        err = 1;
        printf("卡错误 : %s \r\n",buf_uart2.buf);
        Clear_Buffer();
        delay_ms(300);
    }
	
	Clear_Buffer();
    Uart2_SendStr("AT+CGSN=1\r\n");//得到IMEI 国际移动设备识别码
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返OK	
    if(strx)
    {	
        printf("GET IMEI OK\r\n");
        delay_ms(300);
    }
	
	Clear_Buffer();	
    Uart2_SendStr("AT+CGATT?\r\n");//查询附网状态 查询当前模块是否附着到GPRS网络
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGATT: 1");//返1 表示已附着 返0未附着
    errcount = 0;
    while(strx==NULL)
    {
        errcount++;
        Clear_Buffer();	
        Uart2_SendStr("AT+CGATT?\r\n");//获取激活状态
        delay_ms(300);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGATT: 1");//返回1,表明注网成功
        if(errcount>100)     //防止死循环
        {
            err=1;
            errcount = 0;
            break;
        }
    }
//	return err;
	Clear_Buffer();
    Uart2_SendStr("AT+QBAND?\r\n"); //允许错误值 会返回当前工作频段的相关信息 3联通 5电信 8移动
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返回OK
    if(strx)
    {
        printf("========BAND========= \r\n %s \r\n",buf_uart2.buf);
        delay_ms(300);
    }
	
	Clear_Buffer();
    Uart2_SendStr("AT+CSQ\r\n");//查询当前网络信号强度
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"+CSQ");//返回CSQ
	if(strx)
    {
        printf("信号质量:%s\r\n",buf_uart2.buf);//范围：0-31，0表示信号极弱，31表示信号最强；若为99，则表示信号无法检测，比如无SIM卡、无网络
       
        delay_ms(300);
    }
	
	
	Clear_Buffer();
    Uart2_SendStr("AT+CEREG?\r\n");//查询EPS网络注册状态
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"+CEREG: 0,1");//返回注册状态
    extstrx=strstr((const char*)buf_uart2.buf,(const char*)"+CEREG: 1,1");//返回注册状态
    errcount = 0;
    while(strx==NULL&&extstrx==NULL)//两个返回值都没有
    {
        errcount++;
        
		Clear_Buffer();
        Uart2_SendStr("AT+CEREG?\r\n");//判断运营商
        delay_ms(300);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+CEREG: 0,1");//返回注册状态
        extstrx=strstr((const char*)buf_uart2.buf,(const char*)"+CEREG: 1,1");//返回注册状态
        if(errcount>100)     //防止死循环
        {
            err=1;
            errcount = 0;
            break;
        }
    }
    return err;
}

void BC26_PDPACT(void)//激活场景，为连接服务器做准备
{
    int errcount = 0;
    Uart2_SendStr("AT+CGPADDR=1\r\n");//激活场景
    delay_ms(300);
    
	Clear_Buffer();
    Uart2_SendStr("AT+CGSN=1\r\n");//激活场景
    delay_ms(300);
    
	Clear_Buffer();
    Uart2_SendStr("AT+CGATT?\r\n");//激活场景
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGATT: 1");//注册上网络信息	
    while(strx==NULL)
    {
        errcount++;
        Clear_Buffer();
        Uart2_SendStr("AT+CGATT?\r\n");//激活场景
        delay_ms(300);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGATT: 1");//一定要终端入网
        if(errcount>100)     //防止死循环
        {
            errcount = 0;
            break;
        }
    }
    
}


void BC26_RegALIYUNIOT(void)//平台注册
{
    u8  BC20_IMEI[20],i;//IMEI值
	int errcount = 0;
	
    Uart2_SendStr("AT+QMTDISC=0\r\n");//Disconnect a client from MQTT server
    delay_ms(300);
    Clear_Buffer();

    Uart2_SendStr("AT+QMTCLOSE=0\r\n");//删除句柄
    delay_ms(300);
    
	
	Clear_Buffer();
    sprintf(atstr,"AT+QMTCFG=\"version\",0,4\r\n");
	//指定设备与服务器通信的 MQTT 协议版本（确保双方协议兼容，避免连接失败）配置 MQTT 协议版本为 v4
    printf("atstr = %s \r\n",atstr);
    Uart2_SendStr(atstr);//发送阿里云配置参数
    delay_ms(300);  //等待300ms反馈OK
    strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返OK
    while(strx==NULL)
    {
        strx=strstr((const char*)buf_uart2.buf,(const char*)"OK");//返OK
    }
    
	Clear_Buffer();		
	
	sprintf(atstr,"AT+QMTOPEN=0,\"%s\",1883\r\n",ServerIP);
	//建立 TCP 连接
    Uart2_SendStr(atstr);//登录阿里云平台
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"QMTOPEN: 0,0");//返+QMTOPEN: 0,0
    while(strx==NULL)
    {
		errcount++;
		delay_ms(100);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"QMTOPEN: 0,0");//返+QMTOPEN: 0,0
        if(errcount>100)     //防止死循环
        {
			//模块重启
             NVIC_SystemReset();	//没有创建TCP SOCKET就重启系统等到服务器就绪
        }
    }
	
    Clear_Buffer();
	errcount = 0;
    Uart2_SendStr("AT+CGSN=1\r\n");//获取模块的IMEI号
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGSN:");//返+CGSN:
    while(strx==NULL)
    {
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+CGSN:");//返+CGSN:
    }
	
    for(i=0;i<15;i++)BC20_IMEI[i]=strx[i+7];
	
    BC20_IMEI[15]=0;
    
	Clear_Buffer();
    printf("我的模块IMEI是:%s\r\n\r\n\r\n",BC20_IMEI);
    sprintf(atstr,"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",clientId,username,passwd);
	//客户端认证
    printf("atstr = %s \r\n",atstr);//
    Uart2_SendStr(atstr);//发送链接到阿里云
    delay_ms(300);
    strx=strstr((const char*)buf_uart2.buf,(const char*)"QMTCONN: 0,0,0");//返+QMTCONN: 0,0,0
    while(strx==NULL)
    {
        errcount++;
		delay_ms(100);
        strx=strstr((const char*)buf_uart2.buf,(const char*)"QMTCONN: 0,0,0");//QMTCONN: 0,0,0
        if(errcount>100)     //防止死循环
        {
			 Clear_Buffer();
			//模块重启
             NVIC_SystemReset();	//没有创建TCP SOCKET就重启系统等到服务器就绪
        }
    }
	
    Clear_Buffer();
	sprintf(atstr,"AT+QMTSUB=0,1,\"$oc/devices/%s/sys/messages/down\",0\r\n",devid);
    Uart2_SendStr(atstr); //允许错误值
    printf(atstr);
	delay_ms(300);
	strx=strstr((const char*)buf_uart2.buf,(const char*)"+QMTSUB: 0,1,0");//订阅成功
    while(strx==NULL)
    {
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+QMTSUB: 0,1,0");//订阅成功
    }
	if(strx)
    {
        printf("\r\n 平台消息下发主题订阅成功 \r\n");
        delay_ms(50);
    }
    //$oc/devices/{device_id}/sys/properties/set/#
	Clear_Buffer();
	strx = 0;
	sprintf(atstr,"AT+QMTSUB=0,1,\"$oc/devices/%s/sys/commands/#\",0\r\n",devid);
    Uart2_SendStr(atstr); //允许错误值
    printf(atstr);
	delay_ms(300);
	strx=strstr((const char*)buf_uart2.buf,(const char*)"+QMTSUB: 0,1,0");//订阅成功
    while(strx==NULL)
    {
        strx=strstr((const char*)buf_uart2.buf,(const char*)"+QMTSUB: 0,1,0");//订阅成功
    } 
	if(strx)
    {
        printf("\r\n平台消息下发命令订阅成功 \r\n");
        delay_ms(50);
    }
	
}

void BC26_HUAWEIYUNSendData(uint8_t temp,uint8_t humi,uint16_t light,uint16_t soil)
{
	char SendArray[256];																																													//,\"motor\":0								//属性ID																服务ID
	sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"temp\":%d,\"humi\":%d,\"light\":%d,\"soil\":%d}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID",temp,humi,light,soil);
	//上面这行代码的deviID好像填产品ID或者设备ID都可以
	Uart2_SendStr(SendArray); 
//	printf(SendArray);
}

void BC26_RECData(void)
{		
	strx = strstr((const char*)buf_uart2.buf,(const char*)"00");
	if(strx)
	{	
		printf("命令下发\r\n");
		printf(buf_uart2.buf);  //清空前 打印信息
		
		delay_ms(100);
		Clear_Buffer();
	}                                                                          //			buf_uart2.index=0;

		strx=strstr((const char*)buf_uart2.buf,(const char*)"FS:0");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_FS_OFF;
			printf("----------------风扇关闭----------------。\r\n");
//			printf(buf_uart2.buf);			
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"FS\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);			
//			buf_uart2.index=0;
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"FS:1");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_FS_ON;
			printf("----------------风扇打开----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"FS\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"SB:0");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_SB_OFF;
			printf("----------------水泵关闭----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"SB\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"SB:1");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_SB_ON;
			printf("----------------水泵打开----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"SB\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"HOT:1");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_HOT_ON;
			printf("----------------加热片打开----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"HOT\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"HOT:0");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			RELAY_HOT_OFF;
			printf("----------------加热片关闭----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"HOT\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"LIGHT:1");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			GPIO_SetBits(LED2_PORT,LED2_PIN);
			printf("----------------补光灯打开----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"LIGHT\":1}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
		strx=strstr((const char*)buf_uart2.buf,(const char*)"LIGHT:0");//返回+QMTRECV:，表明接收到服务器发回的数据 开灯1
		if(strx)
		{	
			GPIO_ResetBits(LED2_PORT,LED2_PIN);
			printf("----------------补光灯关闭----------------。\r\n");
			char SendArray[256];	 ;																																																							//属性ID																服务ID
			sprintf(SendArray,"AT+QMTPUB=0,0,0,0,\"$oc/devices/%s/sys/properties/report\",\"{\"services\":[{\"service_id\":\"%s\",\"properties\":{\"LIGHT\":0}}]}\"  \r\n",devid,"CSTXBC26MQTTFWID");
			Uart2_SendStr(SendArray); 
//			printf(SendArray);
//			printf(buf_uart2.buf);
			delay_ms(100);
			Clear_Buffer();
		}
}
