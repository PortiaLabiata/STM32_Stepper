#ifndef IT_H_
#define IT_H_

#include "main.h"

/* IRSs */

void SysTick_Handler(void);
void TIM3_IRQHandler(void);

/* Callbacks */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

#endif