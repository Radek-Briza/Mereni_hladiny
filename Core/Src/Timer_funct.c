/*
 * Timer_funct.c
 *
 *  Created on: 19. 4. 2026
 *      Author: radek
 */

#include "main.h"
#include "tim.h"
#include "timer_if.h"

void delayMicroseconds(uint16_t us){
	HAL_TIM_Base_Start(&htim2);
    uint16_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while ((uint16_t)(__HAL_TIM_GET_COUNTER(&htim2) - start) < us);
    HAL_TIM_Base_Stop(&htim2);
}

uint32_t GetTimerTicks(void){
	return HAL_GetTick();
}

/*
 * */
uint32_t GetMillis(void){
	return TIMER_IF_Convert_Tick2ms(GetTimerTicks());
}

void StartCapture(void){
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_2);
}

void StopCapture(void){
	HAL_TIM_IC_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_Base_Stop(&htim1);
}

uint16_t GetCapture(void){
	if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_CC2))
	{
	    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC2);
	    return  HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);
	}
	return 0;
}
