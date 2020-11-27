
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "DS18B20.h"
#include "syslog.h"
#include "motor.h"

void SystemClock_Config(void);
uint8_t get_board_id(void);

uint8_t Rx_Data[14],Board_Number,Temp_Control,control;
extern uint8_t Mark;
uint8_t StateFlag_1,StateFlag_2,StateFlag_3,StateFlag_4;			//运动方向标志位
volatile uint8_t Motor_1,Motor_2,Motor_3,Motor_4;
volatile uint32_t Counter_1=0,Counter_2=0,Counter_3=0,Counter_4=0;
uint8_t T1_Data[14]={0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1};
uint8_t Advance_2,Advance_3;			//电机2和电机3到位后再多走800步，控制参数

uint32_t LED_Count;
uint8_t LED_Mark;
float Temperature_1;				//金属浴温度

float Motor_frequency;						//电机4 调速参数 频率
uint16_t Load_value;			    		//电机4 调速参数 预分频系数
float Count_parameter=1.6;			  //电机4 调速参数 步数调节

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM5_Init();
    MX_TIM1_Init();
    TIM12_Init(159);
	
    Board_Number = get_board_id();
	
    MX_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1, Rx_Data,14);
	HAL_TIM_Base_Start_IT(&htim5);
    MX_IWDG_Init();

    while(1) {
        if (Mark == 1) {
			if (Rx_Data[3] == 0x0a) {

				if (is_motor_zero(MOTOR_X_AXIS_ID))
					Rx_Data[4]=0xFF;
				else
					Rx_Data[4]=0x00;
				if (is_motor_zero(MOTOR_Z_AXIS_ID))
					Rx_Data[5]=0xFF;
				else
					Rx_Data[5]=0x00;
				if (is_motor_zero(MOTOR_SYRINGE_ID))
					Rx_Data[6]=0xFF;
				else
					Rx_Data[6]=0x00;
				if(is_motor_zero(MOTOR_RECEIVED_ID))
					Rx_Data[7]=0xFF;
				else
					Rx_Data[7]=0x00;

			}
			HAL_Delay(50);
			Mark=0;	
            /*data feedback*/
			HAL_UART_Transmit_DMA(&huart1,Rx_Data,14);

			/******************** 电机控制 开始 *******************/
            /*控制电机1运动 【1-7 电机5】  大注射器*/
			if( Rx_Data[3]==1 ) {
				if ((Rx_Data[4] == 0xFF) || ((Rx_Data[4] == 0x00) && !is_motor_zero(MOTOR_SYRINGE_ID))) {
					if (Rx_Data[4]==0xFF) {
                        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_FWD);
                        StateFlag_1=0xFF;
					} else if( (Rx_Data[4]==0x00) && !is_motor_zero(MOTOR_SYRINGE_ID)) {

                        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_REV);
						StateFlag_1=0x00;
					}							
					Counter_1=2*( ((uint32_t)Rx_Data[5]<<16) + ((uint32_t)Rx_Data[6]<<8) + Rx_Data[7] );		//取步数
					Motor_1=1;										//启动电机1		
				}
#if 0
                /*A1表示正转；B1表示停止*/
                else if ((Rx_Data[4] == 0xA1) || (Rx_Data[4] == 0xB1)) {
                  /*A1表示正转*/
                    if(Rx_Data[4]==0xA1) {
                        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_FWD);
                        /*电机1 PWM输出启动函数*/
                        HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
			            /*B1表示停止*/
                    } else if(Rx_Data[4]==0xB1) {
                        /*电机1 停止输出PWM*/
                        HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);
					} else {

                    }
			    }
#endif
            }
            /*控制电机2运动 【1-7 电机4】  X1*/
			else if( Rx_Data[3]==2 ) {
				if  (((Rx_Data[4] == 0xFF) || ((Rx_Data[4] == 0x00) && !is_motor_zero(MOTOR_X_AXIS_ID))) && is_motor_zero(MOTOR_Z_AXIS_ID))
				{
					if(Rx_Data[4] == 0xFF) {

                        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_FWD);
						StateFlag_2=0xFF;
					}
					else if( (Rx_Data[4]==0x00) && (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==1) )
					{
                        set_motor_direction(MOTOR_SYRINGE_ID, DIRECTION_REV);
						StateFlag_2=0x00;
                        /*使能"进800"的功能*/
						Advance_2=1;
					}							
					Counter_2=( ((uint32_t)Rx_Data[5]<<16) + ((uint32_t)Rx_Data[6]<<8) + Rx_Data[7] );
					Motor_2=1;
				}
#if 0
                else if( (Rx_Data[4]==0xA1) || (Rx_Data[4]==0xB1))					//A1表示正转；B1表示停止
				{
					if(Rx_Data[4]==0xA1)			//A1表示正转
					{
						HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);				//方向控制引脚置1表示正向
						HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);									//电机2 PWM输出启动函数						
					}
					else if(Rx_Data[4]==0xB1)			//B1表示停止
					{
						HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);					//电机2 停止输出PWM
					}					
				}
#endif
                /*3号 Z1电机 不在零位发送A1信号*/
				else if (!is_motor_zero(MOTOR_Z_AXIS_ID)) {
					HAL_Delay(500);
                    HAL_UART_Transmit_DMA(&huart1,T1_Data,14);
				}
			}
            /*控制电机3运动 【1-7 电机1】  Z1*/
			else if (Rx_Data[3] == 3) {
				if ((Rx_Data[4]==0xFF) || ((Rx_Data[4]==0x00) && !is_motor_zero(MOTOR_Z_AXIS_ID))) {
                    /*方向选择*/
					if(Rx_Data[4]==0xFF) {
                        set_motor_direction(MOTOR_Z_AXIS_ID, DIRECTION_FWD);
						StateFlag_3=0xFF;
					}
					else if( (Rx_Data[4]==0x00) && (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)==1) )
					{
                        set_motor_direction(MOTOR_Z_AXIS_ID, DIRECTION_REV);
						StateFlag_3=0x00;
						Advance_3=1;					//使能"进800"的功能
					}
					Counter_3=( ((uint32_t)Rx_Data[5]<<16) + ((uint32_t)Rx_Data[6]<<8) + Rx_Data[7] );
					Motor_3=1;
				}
#if 0
                /*A1表示正转；B1表示停止*/
				else if( (Rx_Data[4]==0xA1) || (Rx_Data[4]==0xB1) ) {
                    /*A1表示正转*/
					if(Rx_Data[4]==0xA1)
					{
                        set_motor_direction(MOTOR_Z_AXIS_ID, DIRECTION_FWD);
						HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);								//电机3 PWM输出启动函数
					}
					else if(Rx_Data[4]==0xB1)			//B1表示停止
					{
						HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3);									//电机3 停止输出PWM
					}					
				}				
#endif
			}
			
			else if( Rx_Data[3]==4 )				//控制电机4运动 【1-7 电机7】  预留
			{
				if( (Rx_Data[4]==0xFF) || ( (Rx_Data[4]==0x00) && !is_motor_zero(MOTOR_RECEIVED_ID)))
				{
                    //方向选择
					if(Rx_Data[4]==0xFF) {
                        set_motor_direction(MOTOR_RECEIVED_ID, DIRECTION_FWD);
						StateFlag_4=0xFF;
					}
					else if( (Rx_Data[4]==0x00) && (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)==1) )
					{
                        set_motor_direction(MOTOR_RECEIVED_ID, DIRECTION_REV);
						StateFlag_4=0x00;
					}
					Counter_4=Count_parameter*( ((uint32_t)Rx_Data[5]<<16) + ((uint32_t)Rx_Data[6]<<8) + Rx_Data[7] );		//取步数
					Motor_4=1;																								//启动电机4
				}
//				else if( (Rx_Data[4]==0xA1) || (Rx_Data[4]==0xB1) )					//A1表示正转；B1表示停止
//				{
//					if(Rx_Data[4]==0xA1)			//A1表示正转
//					{
//						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);				//方向控制引脚置1表示正向
//						HAL_TIM_PWM_Start(&htim12,TIM_CHANNEL_1);									//电机4 PWM输出启动函数
//					}
//					else if(Rx_Data[4]==0xB1)			//B1表示停止
//					{
//						HAL_TIM_PWM_Stop(&htim12,TIM_CHANNEL_1);									//电机4 停止输出PWM
//					}					
//				}				
			}
			/******************** 电机控制 结束 *******************/
			
			/******************** 阀门控制 开始 *******************/
            else if ( Rx_Data[3]==5 )
			{
				if(Rx_Data[4]==0xFF)
                    set_valve_state(VALVE_1, VALVE_STATE_OPEN);
				else
                    set_valve_state(VALVE_1, VALVE_STATE_CLOSE);
			}
			else if( Rx_Data[3]==6 )	
			{
				if(Rx_Data[4]==0xFF)
                    set_valve_state(VALVE_2, VALVE_STATE_OPEN);
				else
                    set_valve_state(VALVE_2, VALVE_STATE_CLOSE);
			}		
			else if( Rx_Data[3]==7 )	
			{
				if(Rx_Data[4]==0xFF)
                    set_valve_state(VALVE_3, VALVE_STATE_OPEN);
				else
                    set_valve_state(VALVE_3, VALVE_STATE_CLOSE);
			}		
			else if( Rx_Data[3]==8 )
			{
				if(Rx_Data[4]==0xFF)
                    set_valve_state(VALVE_4, VALVE_STATE_OPEN);
				else
                    set_valve_state(VALVE_4, VALVE_STATE_CLOSE);
			}
			else if( Rx_Data[3]==9 ) {
				if(Rx_Data[4]==0xFF)
                    set_valve_state(VALVE_5, VALVE_STATE_OPEN);
				else
                    set_valve_state(VALVE_5, VALVE_STATE_CLOSE);
			}
#if 0
			else if (Rx_Data[3]==10) {
				if(Rx_Data[4]==0xFF)
                  set_valve_state(VALVE_6, VALVE_STATE_OPEN);
				else
                  set_valve_state(VALVE_6, VALVE_STATE_CLOSE);
			}
#endif
			/******************** 阀门控制 结束 *******************/
			
			/******************** 温度控制使能 开始 *******************/
			else if (Rx_Data[3]==11) {
				if (Rx_Data[4]==0xFF)					//表示开启温控
				{
					Temp_Control=1;
					control=1;
				}
				else if(Rx_Data[4]==0x00)			//表示关闭温控
				{
					Temp_Control=0;	
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);		//停止加热
				}
								
			}
			/******************** 温度控制使能 结束 *******************/
			
			/******************** 四号电机调速程序 开始 *******************/
			else if( (Rx_Data[3]==12) && (Rx_Data[4]<=80) )
			{
				Motor_frequency=Rx_Data[4]/4;						//电机频率
				Load_value=2000/Motor_frequency;  			//重装载值
				TIM12_Init(Load_value);									//设置转速
				Count_parameter=20/Motor_frequency;			//计算步数倍数
			}
			/******************** 四号电机调速程序 结束 *******************/
			
			/******************** 紧急停止程序 开始 *******************/
			else if(Rx_Data[3]==13)
			{
                set_motor_run_stop(MOTOR_SYRINGE_ID, MOTOR_STOP);
				Motor_1=0;
				Counter_1=0;

                set_motor_run_stop(MOTOR_X_AXIS_ID, MOTOR_STOP);
				Motor_2=0;
				Counter_2=0;

                set_motor_run_stop(MOTOR_Z_AXIS_ID, MOTOR_STOP);
				Motor_3=0;
				Counter_3=0;

                set_motor_run_stop(MOTOR_RECEIVED_ID, MOTOR_STOP);
				Motor_4=0;
				Counter_4=0;
				
				/******** 电机4初始化 ********/
				TIM12_Init(159);
				Count_parameter=1.6;
				
				/******** 阀门关闭 ********/
                for (int i = VALVE_1; i < VALVE_END; i++) {
                    set_valve_state(i, VALVE_STATE_CLOSE); 
                }
			}
			/******************** 紧急停止程序 结束 *******************/
		}	


		/************ 步数无法清零应对程序 开始 ************/
        if(Motor_1==0) {
            set_motor_run_stop(MOTOR_SYRINGE_ID, MOTOR_STOP);
            Counter_1=0;
        }
        if(Motor_2==0) {
            set_motor_run_stop(MOTOR_X_AXIS_ID, MOTOR_STOP);
            Counter_2=0;
        }
        if(Motor_3==0) {
            set_motor_run_stop(MOTOR_Z_AXIS_ID, MOTOR_STOP);
            Counter_3=0;
        }
        if(Motor_4==0) {
            set_motor_run_stop(MOTOR_RECEIVED_ID, MOTOR_STOP);
            Counter_4=0;
        }			
		/************ 步数无法清零应对程序 结束 ************/	
			
		/************ 归零后避免撞击程序 开始（可以增加一字节，发送归零信号）************/
		if ((StateFlag_1 == 0x00) && (is_motor_zero(MOTOR_SYRINGE_ID)) && (Counter_1 > 10))				//电机1到位后如果步数异常则进行校正
		{
            set_motor_run_stop(MOTOR_SYRINGE_ID, MOTOR_STOP);
			Motor_1=0;
			Counter_1=0;
		}

		if( (StateFlag_2==0x00) && (is_motor_zero(MOTOR_X_AXIS_ID)) && (Counter_2 > 2000) && (Advance_2==0))
		{
            set_motor_run_stop(MOTOR_X_AXIS_ID, MOTOR_STOP);
			Motor_2=0;
			Counter_2=0;			
		}

		if( (StateFlag_3==0x00) && (is_motor_zero(MOTOR_Z_AXIS_ID)) && (Counter_3 > 2000) && (Advance_3==0))
		{
            set_motor_run_stop(MOTOR_Z_AXIS_ID, MOTOR_STOP);
			Motor_3=0;
			Counter_3=0;
		}	

		if( (StateFlag_4==0x00) && (is_motor_zero(MOTOR_RECEIVED_ID)) && (Counter_4 > 10))				//电机4到位后如果步数异常则进行校正
		{
            set_motor_run_stop(MOTOR_RECEIVED_ID, MOTOR_STOP);
			Motor_4=0;
			Counter_4=0;
		}

		/************ 归零后避免撞击程序 结束（可以增加一字节，发送归零信号）************/		
		LED_Count++;
		if (LED_Count%1655990 == 0)
		{
		/******************** 温度控制 开始 *******************/			
			if (Temp_Control == 1) {
				Temperature_1 = DS18B20_GetTemp_SkipRom(1);						//取金属浴温度
				if (control == 1) {
					if (Temperature_1 < 80)
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);			//开始加热
					else
					{
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);		//停止加热
						control=0;
					}				
				}
				else if(Temperature_1<45)
					control=1;
			}
		/******************** 温度控制 结束 *******************/
			
		/********* LED灯闪烁 开始 **********/		
			LED_Count=0;
 
			if(LED_Mark==0)
			{
				LED_Mark=1;
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
			}
			else
			{
				LED_Mark=0;
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
			}			
		/********* LED灯闪烁 结束 **********/		
		}
		HAL_IWDG_Refresh(&hiwdg);				//看门狗			
  }
}

/************************* 定时器中断回调函数  开始 *************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)	
{
	if (htim == &htim5) {
		if (Motor_1 == 1)							//电机1 大注射器 启停控制程序
		{
			if ((Counter_1 >= 1) && ((StateFlag_1 == 0xFF) || ((StateFlag_1 == 0x00) && (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4)==1))))
			{
				HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);				//电机1 PWM输出启动函数
				Counter_1--;																		//步数递减计数
			}
			else
			{
				HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);					//电机1 停止输出PWM
				Motor_1=0;
				Counter_1=0;
			}
		}

		if(Motor_2==1)							//电机2 X1 启停控制程序   
		{
//			if( (Counter_2>=1) && ((StateFlag_2==0xFF) || ((StateFlag_2==0x00) && (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==1))) )
//			{
//				HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);				//电机2 PWM输出启动函数
//				Counter_2--;																		//步数递减计数
//			}
//			else
//			{
//				HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);					//电机2 停止输出PWM
//				Motor_2=0;
//				Counter_2=0;
//			}
			if(Counter_2>=1)
			{
				HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);				//电机2 PWM输出启动函数
				Counter_2--;																		//步数递减计数
			}
			else
			{
				HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);					//电机2 停止输出PWM
				Motor_2=0;
				Counter_2=0;					//电机2 已停止，清除计数器
				Advance_2=0;				
			}
			if(Advance_2==1)
			{
				if( (StateFlag_2==0x00) && (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==0) )
				{
					Counter_2=800;					//电机到达零位,启动"进800"功能
					Advance_2=0;					//清除标志位,本次运动不再进入这个功能
				}				
			}			
			
		}		

		if(Motor_3==1)							//电机3 Z1 启停控制程序   
		{
//			if( (Counter_3>=1) && ((StateFlag_3==0xFF) || ((StateFlag_3==0x00) && (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)==1))) )
//			{
//				HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);				//电机3 PWM输出启动函数
//				Counter_3--;																		//步数递减计数
//			}
//			else
//			{
//				HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3);					//电机3 停止输出PWM
//				Motor_3=0;
//				Counter_3=0;					//电机3 已停止，清除计数器	
//			}
			if(Counter_3>=1)
			{
				HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);				//电机3 PWM输出启动函数
				Counter_3--;																		//步数递减计数
			}
			else
			{
				HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3);					//电机3 停止输出PWM
				Motor_3=0;
				Counter_3=0;
				Advance_3=0;
			}				
			if(Advance_3==1)
			{
				if( (StateFlag_3==0x00) && (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)==0) )
				{
					Counter_3=800;					//电机到达零位,启动"进800"功能
					Advance_3=0;					//清除标志位,本次运动不再进入这个功能
				}
			}
		}

		if(Motor_4==1)							//电机4 预留 启停控制程序   
		{
			if( (Counter_4>=1) && ((StateFlag_4==0xFF) || ((StateFlag_4==0x00) && (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)==1))) )
			{
				HAL_TIM_PWM_Start(&htim12,TIM_CHANNEL_1);				//电机4 PWM输出启动函数
				Counter_4--;																		//步数递减计数
			}
			else
			{
				HAL_TIM_PWM_Stop(&htim12,TIM_CHANNEL_1);					//电机4 停止输出PWM
				Motor_4=0;
				Counter_4=0;					//电机4 已停止，清除计数器	
			}
		}		
	}
}
/************************* 定时器中断回调函数  结束 *************************/

uint8_t get_board_id(void)
{
    uint8_t id = 0;
    GPIO_PinState b4 = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
    GPIO_PinState b3 = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1);
    GPIO_PinState b2 = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2);
    GPIO_PinState b1 = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3);
    GPIO_PinState b0 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3);
    id = ((b4 << 4)|(b3 << 3)|(b2 << 2)|(b1 << 1)|b0);
    LOG_I("Board id %d", id);
    return id;
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
