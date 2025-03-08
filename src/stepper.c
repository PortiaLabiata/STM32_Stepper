#include "stepper.h"

/* Pin configurations */

static uint32_t PinConf_Wavemode[4] = {
    0b1000L,
    0b0100L,
    0b0010L,
    0b0001L
};
  
static uint32_t PinConf_Stepmode[4] = {
    0b1001L,
    0b1100L,
    0b0110L,
    0b0011L
};
  
static uint32_t PinConf_HalfStepmode[7] = {
    0b1000L,
    0b1100L,
    0b0100L,
    0b0110L,
    0b0010L,
    0b0001L,
    0b1001L
};

/* Global variables */

GPIO_InitTypeDef GPIO_InitStruct;
TIM_HandleTypeDef htim3;
/* Это временная мера, чтобы можно было проверять работоспособность программы, 
  пока я не придумаю нормальный способ. */
static Stepper_InitStruct_t *__stepper;

/* Control functions */

void Stepper_Init(Stepper_InitStruct_t *stepper)
{
  TIM3_Config(TIM3, stepper->period, stepper->presc);
  GPIO_Config(stepper->gpios);
  stepper->__state = STEPPER_STATE_READY;
  __stepper = stepper;
}

void Stepper_Step(Stepper_InitStruct_t *stepper, int steps, StepperDirec direc, StepperModes mode)
{
    volatile uint8_t size_;

    switch (mode)
    {
        case STEPPER_MODE_WAVE:
            stepper->__buffer.array = PinConf_Wavemode;
            size_ = 4;
            break;
        case STEPPER_MODE_STEP:
            stepper->__buffer.array = PinConf_Stepmode;
            size_ = 4;
            break;
        case STEPPER_MODE_HALFSTEP:
            stepper->__buffer.array = PinConf_HalfStepmode;
            size_ = 7;
            break;
    }
    stepper->__buffer.size = size_;
    stepper->__buffer.index = !direc ? 0 : size_-1;
    stepper->__steps_left = steps;
    stepper->__direc = direc;
    stepper->__mode = mode;
    HAL_TIM_Base_Start_IT(&htim3);
    Stepper_SetState(stepper, STEPPER_STATE_RUNNING); 
}

void Stepper_Halt(Stepper_InitStruct_t *stepper)
{
  Stepper_SetState(stepper, STEPPER_STATE_HALTED);
}

void Stepper_Resume(Stepper_InitStruct_t *stepper)
{
  Stepper_SetState(stepper, STEPPER_STATE_RUNNING);
}

void Stepper_PollForFinish(Stepper_InitStruct_t *stepper)
{
  while (Stepper_GetState(stepper) == STEPPER_STATE_RUNNING) __asm("");
}

/* Getters and setters */

StepperModes Stepper_GetMode(Stepper_InitStruct_t stepper)
{
  return stepper.__mode;
}

StepperDirec Stepper_GetDirec(Stepper_InitStruct_t stepper)
{
  return stepper.__direc;
}

Stepper_State Stepper_GetState(Stepper_InitStruct_t *stepper)
{
  return stepper->__state;
}

void Stepper_SetState(Stepper_InitStruct_t *stepper, Stepper_State state)
{
  stepper->__state = state;
}

/* Interrupt handlers and callbacks */

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM3) {
      if (Stepper_GetState(__stepper) != STEPPER_STATE_RUNNING) return;

      GPIOA->ODR &= ~GPIO_MASK;
      if (__stepper->__steps_left == 0) {
          HAL_TIM_Base_Stop_IT(&htim3);
          Stepper_SetState(__stepper, STEPPER_STATE_READY);
          return;
      }

      volatile uint32_t next_conf;
      if (__stepper->__direc == STEPPER_DIRECTION_FORWARD)
          next_conf = CircBuffer_Next(&__stepper->__buffer);
      else if (__stepper->__direc == STEPPER_DIRECTION_REVERSE)
          next_conf = CircBuffer_Prev(&__stepper->__buffer);

      GPIOA->ODR |= next_conf;
      if (__stepper->__steps_left != RUN_INDEFINETLY) __stepper->__steps_left--;
    }
}

/* Peripherals configuration */

void TIM3_Config(TIM_TypeDef *tim, uint16_t period, uint32_t presc)
{
  htim3.Instance = tim;
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

void GPIO_Config(uint16_t *iPins)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  for (int i = 0; i < 4; i++) {
    GPIO_InitStruct.Pin = iPins[i];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}