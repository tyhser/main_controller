#ifndef __ONEWIRE_DS18B20_H__
#define __ONEWIRE_DS18B20_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
HAL_StatusTypeDef DS18B20_Init(uint8_t channel);
void DS18B20_ReadId(uint8_t channel, uint8_t * ds18b20_id);
float DS18B20_GetTemp_SkipRom(uint8_t channel);
float DS18B20_GetTemp_MatchRom(uint8_t channel, uint8_t *ds18b20_id);

#endif /* __ONEWIRE_DS18B20_H__ */
/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/

