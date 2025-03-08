#include "main.h"

/* Configuration */

void SystemClock_Config(void);
void TIM3_Config(uint16_t period, uint32_t presc);
void GPIO_Config(void);

/* Msp Configuration */

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim);
