#include "stm32f10x_it.h"
#include "circ_buffer.h"

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  Stepper_SingleStep(htim);
}