#include "stepper.h"

/* Pin configurations */

uint32_t PinConf_Wavemode[4] = {
    0b1000L,
    0b0100L,
    0b0010L,
    0b0001L
};
  
uint32_t PinConf_Stepmode[4] = {
    0b1100L,
    0b0110L,
    0b0011L,
    0b1001L
};
  
uint32_t PinConf_HalfStepmode[7] = {
    0b1000L,
    0b1100L,
    0b0100L,
    0b0110L,
    0b0010L,
    0b0001L,
    0b1001L
};

/* Global variables */

circular_buffer_t current_buffer;

uint32_t iCurrentPin_Conf;
StepperModes current_mode;
StepperDirec current_direc;
uint32_t steps_left = 0;

GPIO_InitTypeDef GPIO_InitStruct;
TIM_HandleTypeDef htim3;

/* Functions */

void Stepper_Init(uint16_t period, uint32_t presc)
{
  TIM3_Config(period, presc);
  GPIO_Config();
}

void Stepper_Step(int steps, StepperDirec direc, StepperModes mode)
{
    current_direc = direc;
    current_mode = mode;
    volatile uint8_t size_;

    switch (mode)
    {
        case STEPPER_MODE_WAVE:
            current_buffer.array = PinConf_Wavemode;
            size_ = 4;
            break;
        case STEPPER_MODE_STEP:
            current_buffer.array = PinConf_Stepmode;
            size_ = 4;
            break;
        case STEPPER_MODE_HALFSTEP:
            current_buffer.array = PinConf_HalfStepmode;
            size_ = 7;
            break;
    }
    current_buffer.size = size_;
    current_buffer.index = !direc ? 0 : size_-1;
    steps_left = (mode != STEPPER_MODE_HALFSTEP ? steps : steps*2);
    HAL_TIM_Base_Start_IT(&htim3);
}

/* Interrupt handlers and callbacks */

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM3) {
        GPIOA->ODR &= ~GPIO_MASK;
        if (steps_left == 0) {
            HAL_TIM_Base_Stop_IT(&htim3);
            return;
        }
        
        volatile uint32_t next_conf;
        if (current_direc == STEPPER_DIRECTION_FORWARD)
            next_conf = CircBuffer_Next(&current_buffer);
        else if (current_direc == STEPPER_DIRECTION_REVERSE)
            next_conf = CircBuffer_Prev(&current_buffer);

        GPIOA->ODR |= next_conf;
        if (steps_left != RUN_INDEFINETLY) steps_left--;
    }
}

void TIM3_Config(uint16_t period, uint32_t presc)
{
  htim3.Instance = TIM3;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = period;
  htim3.Init.Prescaler = presc;
  htim3.Init.RepetitionCounter = 0;
  
  HAL_TIM_Base_Init(&htim3);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
  if (htim->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }

  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void GPIO_Config(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  uint16_t iPins[4] = {
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3
  };

  for (int i = 0; i < 4; i++) {
    GPIO_InitStruct.Pin = iPins[i];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}