#ifndef STEP_H_
#define STEP_H_

#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_tim.h>
#include <stm32f103x6.h>

#include <stdint.h>

#include "circ_buffer.h"

/* Magic numbers */

#define GPIO_MASK 0b1111L;
#define RUN_INDEFINETLY -1

typedef enum {
    STEPPER_MODE_WAVE,
    STEPPER_MODE_STEP,
    STEPPER_MODE_HALFSTEP
} StepperModes;

typedef enum {
    STEPPER_DIRECTION_FORWARD,
    STEPPER_DIRECTION_REVERSE
} StepperDirec;

void Stepper_Init(uint16_t period, uint32_t presc);
void Stepper_Step(int steps, StepperDirec direc, StepperModes mode);

void TIM3_Config(uint16_t period, uint32_t presc);
void GPIO_Config(void);

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim);

#endif