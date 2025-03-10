#include "stepper.h"

/* Pin configurations */

static const uint32_t PinConf_Wavemode[4] = {
    0b1000L,
    0b0100L,
    0b0010L,
    0b0001L
};
  
static const uint32_t PinConf_Stepmode[4] = {
    0b1001L,
    0b1100L,
    0b0110L,
    0b0011L
};
  
static const uint32_t PinConf_HalfStepmode[7] = {
    0b1000L,
    0b1100L,
    0b0100L,
    0b0110L,
    0b0010L,
    0b0001L,
    0b1001L
};

/* Global variables */

static Stepper_InitStruct_t *steppers[MAX_STEPPERS];
static uint32_t steppers_num = 0;

/* Control functions */

ConfigState Stepper_Init(Stepper_InitStruct_t *stepper)
{
  if (stepper == NULL) return STEPPER_ERROR_CONFIG;
  if (steppers_num > MAX_STEPPERS) return STEPPER_ERROR_CONFIG;

  // TIM3_Config(TIM3, stepper->period, stepper->presc);
  // GPIO_Config(stepper->gpios);

  stepper->__state = STEPPER_STATE_READY;
  steppers[steppers_num++] = stepper;
  return STEPPER_OK;
}

ConfigState Stepper_Step(Stepper_InitStruct_t *stepper, int steps, StepperDirec direc, StepperModes mode)
{
    if (stepper == NULL ) return STEPPER_ERROR_CONTROL;
    if (direc != STEPPER_DIRECTION_FORWARD || direc != STEPPER_DIRECTION_REVERSE) 
      return STEPPER_INVALID_DIREC;
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
        default:
          return STEPPER_INVALID_MODE;
    }

    stepper->__buffer.size = size_;
    stepper->__buffer.index = !direc ? 0 : size_-1;
    stepper->__steps_left = steps;
    stepper->__direc = direc;
    stepper->__mode = mode;

    if (HAL_TIM_Base_Start_IT(stepper->htim) != HAL_OK) return STEPPER_ERROR_CONTROL;
    Stepper_SetState(stepper, STEPPER_STATE_RUNNING); 
}

ConfigState Stepper_Pause(Stepper_InitStruct_t *stepper, Stepper_State hold)
{
  if (hold != STEPPER_STATE_HOLDING || hold != STEPPER_STATE_FREE) 
    return STEPPER_ERROR_CONTROL;
  Stepper_SetState(stepper, hold);
  return STEPPER_OK;
}

ConfigState Stepper_Halt(Stepper_InitStruct_t *stepper)
{
  if (HAL_TIM_Base_Stop_IT(stepper->htim) != HAL_OK) 
    return STEPPER_ERROR_CONTROL;
  Stepper_SetState(stepper, STEPPER_STATE_READY);
  return STEPPER_OK;
}

void Stepper_Resume(Stepper_InitStruct_t *stepper)
{
  Stepper_SetState(stepper, STEPPER_STATE_RUNNING);
}

ConfigState Stepper_PollForFinish(Stepper_InitStruct_t *stepper)
{
  if (stepper == NULL) return STEPPER_ERROR_CONTROL;
  while (Stepper_GetState(*stepper) == STEPPER_STATE_RUNNING) __asm("");
  return STEPPER_OK;
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

Stepper_State Stepper_GetState(Stepper_InitStruct_t stepper)
{
  return stepper.__state;
}

ConfigState Stepper_SetState(Stepper_InitStruct_t *stepper, Stepper_State state)
{
  if (state <= (uint8_t)STEPPER_OK || state >= (uint8_t)STEPPER_ERROR_CONFIG) 
    return STEPPER_ERROR_CONTROL;
  stepper->__state = state;
  return STEPPER_OK;
}

/* Hardware functions */

ConfigState Stepper_SingleStep(TIM_HandleTypeDef *htim)
{
  Stepper_InitStruct_t *stepper = NULL;
  for (int i = 0; i <= MAX_STEPPERS; i++) {
    if (steppers[i] == NULL) break;
    if (steppers[i]->htim->Instance == htim->Instance) stepper = steppers[i];
  }
  if (stepper == NULL) return STEPPER_ERROR_CONTROL;

  /* Не уверен, хорошее ли это решение, всё таки тут целых два обращения к структуре,
    да ещё и два захода в функцию */
  if (Stepper_GetState(*stepper) == STEPPER_STATE_FREE) {
    GPIOA->ODR &= ~GPIO_MASK;
    return STEPPER_OK;
  } else if (Stepper_GetState(*stepper) == STEPPER_STATE_HOLDING) {
    return STEPPER_OK;
  }

  if (stepper->__steps_left == 0) {
      if (HAL_TIM_Base_Stop_IT(stepper->htim) != HAL_OK) return STEPPER_ERROR_CONTROL;
      Stepper_SetState(stepper, STEPPER_STATE_READY);
      return STEPPER_OK;
  }

  if (stepper->__direc == STEPPER_DIRECTION_FORWARD)
      GPIOA->ODR |= CircBuffer_Next(&stepper->__buffer);
  else if (stepper->__direc == STEPPER_DIRECTION_REVERSE)
      GPIOA->ODR |= CircBuffer_Prev(&stepper->__buffer);

  if (stepper->__steps_left != RUN_INDEFINITELY) stepper->__steps_left--;
  return STEPPER_OK;
}