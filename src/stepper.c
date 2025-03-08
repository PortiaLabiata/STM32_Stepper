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
  if (steppers_num >= MAX_STEPPERS) return STEPPER_ERROR;

  // TIM3_Config(TIM3, stepper->period, stepper->presc);
  // GPIO_Config(stepper->gpios);

  stepper->__state = STEPPER_STATE_READY;
  stepper = stepper;
  steppers[steppers_num++] = stepper;
  return STEPPER_OK;
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

    HAL_TIM_Base_Start_IT(stepper->htim);
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
  while (Stepper_GetState(*stepper) == STEPPER_STATE_RUNNING) __asm("");
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

void Stepper_SetState(Stepper_InitStruct_t *stepper, Stepper_State state)
{
  stepper->__state = state;
}

/* Hardware functions */

void Stepper_SingleStep(TIM_HandleTypeDef *htim)
{
  Stepper_InitStruct_t *stepper = {0};
  for (int i = 0; i <= MAX_STEPPERS; i++) {
    if (steppers[i] == NULL) break;
    if (steppers[i]->htim->Instance == htim->Instance) stepper = steppers[i];
  }

  if (Stepper_GetState(*stepper) != STEPPER_STATE_RUNNING) return;

  GPIOA->ODR &= ~GPIO_MASK;
  if (stepper->__steps_left == 0) {
      HAL_TIM_Base_Stop_IT(stepper->htim);
      Stepper_SetState(stepper, STEPPER_STATE_READY);
      return;
  }

  if (stepper->__direc == STEPPER_DIRECTION_FORWARD)
      GPIOA->ODR |= CircBuffer_Next(&stepper->__buffer);
  else if (stepper->__direc == STEPPER_DIRECTION_REVERSE)
      GPIOA->ODR |= CircBuffer_Prev(&stepper->__buffer);

  if (stepper->__steps_left != RUN_INDEFINETLY) stepper->__steps_left--;
}