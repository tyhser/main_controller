#include "DS18B20.h"

//#define Delay_ms(x)   HAL_Delay(x)

#define HSI_FREQ 			((uint32_t)168000000)
//#define HSI_VALUE    ((uint32_t)8000000)
#define DELAY_US 			(168000000/32)

static void HAL_DelayuS(uint16_t time)
{
	//当前函数以48MHz运行时，750实际运行时间为764.3uS
  while(time)
  {    
		for (uint8_t i = 0; i < (HSI_FREQ/DELAY_US); i++);
		time--;
  }
}

static void DS18B20_Mode_IPU(uint8_t channel)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
	
		 if(channel==1)  { GPIO_InitStruct.Pin   = TEMP1_Pin;HAL_GPIO_Init(TEMP1_GPIO_Port, &GPIO_InitStruct); }
else if(channel==2)	 { GPIO_InitStruct.Pin   = TEMP2_Pin;HAL_GPIO_Init(TEMP2_GPIO_Port, &GPIO_InitStruct); }
//else if(channel==3)	 { GPIO_InitStruct.Pin   = TEMP3_Pin;HAL_GPIO_Init(TEMP3_GPIO_Port, &GPIO_InitStruct); }  
}

static void DS18B20_Mode_Out_PP(uint8_t channel)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
		 if(channel==1)  { 
		 GPIO_InitStruct.Pin   = TEMP1_Pin;HAL_GPIO_Init(TEMP1_GPIO_Port, &GPIO_InitStruct); }
else if(channel==2)	 { GPIO_InitStruct.Pin   = TEMP2_Pin;HAL_GPIO_Init(TEMP2_GPIO_Port, &GPIO_InitStruct); }
//else if(channel==3)	 { GPIO_InitStruct.Pin   = TEMP3_Pin;HAL_GPIO_Init(TEMP3_GPIO_Port, &GPIO_InitStruct); }  
}

static GPIO_PinState DS18B20_Data_IN(uint8_t channel)
{
	GPIO_PinState bitstatus;
		 if(channel==1)  bitstatus=HAL_GPIO_ReadPin(TEMP1_GPIO_Port, TEMP1_Pin);
else if(channel==2)	 bitstatus=HAL_GPIO_ReadPin(TEMP2_GPIO_Port, TEMP2_Pin);
//else if(channel==3)	 bitstatus=HAL_GPIO_ReadPin(TEMP3_GPIO_Port, TEMP3_Pin);
	return bitstatus;
//	return HAL_GPIO_ReadPin(TEMP1_GPIO_Port, TEMP1_Pin);
}

static void DS18B20_Dout_HIGH(uint8_t channel)
{
  			 if(channel==1)  
				 HAL_GPIO_WritePin(TEMP1_GPIO_Port, TEMP1_Pin, GPIO_PIN_SET);
  	else if(channel==2)	 HAL_GPIO_WritePin(TEMP2_GPIO_Port, TEMP2_Pin, GPIO_PIN_SET);
//		else if(channel==3)	 HAL_GPIO_WritePin(TEMP3_GPIO_Port, TEMP3_Pin, GPIO_PIN_SET);
}

static void DS18B20_Dout_LOW(uint8_t channel)
{
  			 if(channel==1)  
				 HAL_GPIO_WritePin(TEMP1_GPIO_Port, TEMP1_Pin, GPIO_PIN_RESET);
  	else if(channel==2)	 HAL_GPIO_WritePin(TEMP2_GPIO_Port, TEMP2_Pin, GPIO_PIN_RESET);
//  	else if(channel==3)	 HAL_GPIO_WritePin(TEMP3_GPIO_Port, TEMP3_Pin, GPIO_PIN_RESET);
}

// 函数功能: 主机给从机发送复位脉冲
static void DS18B20_Rst(uint8_t channel)
{
	/* 主机设置为推挽输出 */
	DS18B20_Mode_Out_PP(channel);

	DS18B20_Dout_LOW(channel);

	/* 主机至少产生480us的低电平复位信号 */
	HAL_DelayuS(500);

	/* 主机在产生复位信号后，需将总线拉高 */
	DS18B20_Mode_IPU(channel);

	/*从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲*/
	HAL_DelayuS(15);
}

/**
  * 函数功能: 检测从机给主机返回的存在脉冲
  * 输入参数: 无
  * 返 回 值: 0：成功，1：失败
  * 说    明：无
  */
static HAL_StatusTypeDef DS18B20_Presence(uint8_t channel)
{
	uint8_t pulse_time = 0;

	/* 主机设置为上拉输入 */
	DS18B20_Mode_IPU(channel);
	HAL_DelayuS(2);
	/* 等待15-60us,检测存在脉冲的到来，存在脉冲为一个60~240us的低电平信号 
	 * 如果存在脉冲没有来则做超时处理，从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
	 */
	while( DS18B20_Data_IN(channel) && (pulse_time<100) )
	{
		pulse_time++;
		HAL_DelayuS(1);
	}        
	/* 经过100us后，存在脉冲都还没有到来*/
	if( pulse_time >=100 )
		return HAL_ERROR;
	else
		pulse_time = 0;

	/* 存在脉冲到来，且存在的时间不能超过240us */
	while( (!DS18B20_Data_IN(channel)) && (pulse_time<250) )
	{
					pulse_time++;
					HAL_DelayuS(1);
	}        
	if( pulse_time >=250 )
		return HAL_ERROR;
	else
	{
		HAL_DelayuS(400);
		return HAL_OK;
	}
}

HAL_StatusTypeDef DS18B20_Init(uint8_t channel)
{
	DS18B20_Rst(channel);
  return DS18B20_Presence (channel);
}

/**
  * 函数功能: 从DS18B20读取一个bit
  * 输入参数: 无
  * 返 回 值: 读取到的数据
  * 说    明：无
  */
static uint8_t DS18B20_ReadBit(uint8_t channel)
{
	uint8_t dat;

	/* 读0和读1的时间至少要大于60us */        
	DS18B20_Mode_Out_PP(channel);
	/* 读时间的起始：必须由主机产生 >1us <15us 的低电平信号 */
	DS18B20_Dout_LOW(channel);
	HAL_DelayuS(6);

	/* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
	DS18B20_Mode_IPU(channel);
	HAL_DelayuS(3);
	if( DS18B20_Data_IN(channel) == GPIO_PIN_SET )
		dat = 1;
	else
		dat = 0;
	/* 这个延时参数请参考时序图 */
	HAL_DelayuS(55);
	return dat;
}

/**
  * 函数功能: 从DS18B20读一个字节，低位先行
  * 输入参数: 无
  * 返 回 值: 读到的数据
  * 说    明：无
  */
static uint8_t DS18B20_ReadByte(uint8_t channel)
{
	uint8_t i, j, dat = 0;        
	
	for(i=0; i<8; i++) 
	{
		j = DS18B20_ReadBit(channel);                
		dat = (dat) | (j<<i);
	}
	return dat;
}

/**
  * 函数功能: 写一个字节到DS18B20，低位先行
  * 输入参数: dat：待写入数据
  * 返 回 值: 无
  * 说    明：无
  */
static void DS18B20_WriteByte(uint8_t channel, uint8_t dat)
{
	DS18B20_Mode_Out_PP(channel);
	for(uint8_t i=0; i<8; i++ )
	{              
		/* 写0和写1的时间至少要大于60us */
		if ( (0x01<<i)&dat )
		{                        
			DS18B20_Dout_LOW(channel);
			/* 1us < 这个延时 < 15us */
			HAL_DelayuS(5);

			DS18B20_Dout_HIGH(channel);
			HAL_DelayuS(64);
		}                
		else
		{                        
			DS18B20_Dout_LOW(channel);
			/* 60us < Tx 0 < 120us */
			HAL_DelayuS(60);
			
			DS18B20_Dout_HIGH(channel);                
			/* 1us < Trec(恢复时间) < 无穷大*/
			HAL_DelayuS(9);
		}
	}
}

/**
  * 函数功能: 跳过匹配 DS18B20 ROM
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void DS18B20_SkipRom (uint8_t channel)
{
	DS18B20_Rst(channel);                   
	DS18B20_Presence(channel);                 
	DS18B20_WriteByte(channel, 0XCC);                /* 跳过 ROM */        
}

/**
  * 函数功能: 执行匹配 DS18B20 ROM
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void DS18B20_MatchRom (uint8_t channel)
{
	DS18B20_Rst(channel);                   
	DS18B20_Presence(channel);                 
	DS18B20_WriteByte(channel, 0X55);                /* 匹配 ROM */        
}


/*
 * 存储的温度是16 位的带符号扩展的二进制补码形式
 * 当工作在12位分辨率时，其中5个符号位，7个整数位，4个小数位
 *
 *         |---------整数----------|-----小数 分辨率 1/(2^4)=0.0625----|
 * 低字节  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
 *
 *
 *         |-----符号位：0->正  1->负-------|-----------整数-----------|
 * 高字节  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
 *
 * 
 * 温度 = 符号位 + 整数 + 小数*0.0625
 */
/**
  * 函数功能: 在跳过匹配 ROM 情况下获取 DS18B20 温度值 
  * 输入参数: 无
  * 返 回 值: 温度值
  * 说    明：无
  */
float DS18B20_GetTemp_SkipRom (uint8_t channel)
{
	uint8_t tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	DS18B20_SkipRom (channel);
	DS18B20_WriteByte(channel,0X44);                                /* 开始转换 */


	DS18B20_SkipRom (channel);
	DS18B20_WriteByte(channel,0XBE);                                /* 读温度值 */

	tplsb = DS18B20_ReadByte(channel);                 
	tpmsb = DS18B20_ReadByte(channel); 


	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;

	if( s_tem < 0 )                /* 负温度 */
					f_tem = (~s_tem+1) * 0.0625;        
	else
					f_tem = s_tem * 0.0625;

	return f_tem;         
}

/**
  * 函数功能: 在匹配 ROM 情况下获取 DS18B20 温度值 
  * 输入参数: ds18b20_id：用于存放 DS18B20 序列号的数组的首地址
  * 返 回 值: 无
  * 说    明：无
  */
void DS18B20_ReadId (uint8_t channel, uint8_t * ds18b20_id )
{
	uint8_t uc;
					
	DS18B20_WriteByte(channel,0x33);       //读取序列号

	for ( uc = 0; uc < 8; uc ++ )
		ds18b20_id [ uc ] = DS18B20_ReadByte(channel);        
}

/**
  * 函数功能: 在匹配 ROM 情况下获取 DS18B20 温度值 
  * 输入参数: ds18b20_id：存放 DS18B20 序列号的数组的首地址
  * 返 回 值: 温度值
  * 说    明：无
  */
float DS18B20_GetTemp_MatchRom (uint8_t channel, uint8_t * ds18b20_id )
{
	uint8_t tpmsb, tplsb, i;
	short s_tem;
	float f_tem;


	DS18B20_MatchRom (channel);            //匹配ROM

	for(i=0;i<8;i++)
					DS18B20_WriteByte (channel, ds18b20_id [ i ] );        

	DS18B20_WriteByte(channel,0X44);                                /* 开始转换 */


	DS18B20_MatchRom (channel);            //匹配ROM

	for(i=0;i<8;i++)
					DS18B20_WriteByte (channel, ds18b20_id [ i ] );        

	DS18B20_WriteByte(channel,0XBE);                                /* 读温度值 */

	tplsb = DS18B20_ReadByte(channel);                 
	tpmsb = DS18B20_ReadByte(channel); 


	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;

	if( s_tem < 0 )                /* 负温度 */
					f_tem = (~s_tem+1) * 0.0625;        
	else
					f_tem = s_tem * 0.0625;

	return f_tem;                 
}

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
