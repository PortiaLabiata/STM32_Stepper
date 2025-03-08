#ifndef MAIN_H_
#define MAIN_H_

#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f103x6.h>

#include "stm32f10x_it.h"
#include "configuration.h"
#include "stepper.h"

/* Magic numbers */

#define POLL_FOR_READINESS() while (HAL_TIM_Base_GetState(&htim3) != HAL_TIM_STATE_READY) ;

/* Types */

/* Global definitions */

extern GPIO_InitTypeDef GPIO_InitStruct;
extern TIM_HandleTypeDef htim3;

extern uint16_t gpios[4];

/* User functions */

void Set_StepperMode(StepperModes mode, StepperDirec direc, uint16_t period);

#endif