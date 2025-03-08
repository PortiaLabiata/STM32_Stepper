#include "main.h"

#define mode_ STEPPER_MODE_HALFSTEP
#define PERIOD 7999U
#define PRESC 9U

GPIO_InitTypeDef GPIO_InitStruct;
TIM_HandleTypeDef htim3;

uint16_t gpios[4] = {GPIO_PIN_0,GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  TIM3_Config(PERIOD, PRESC);
  __HAL_DBGMCU_FREEZE_TIM3();
  GPIO_Config();

  Stepper_InitStruct_t stepper; 
  TIM_HandleTypeDef htim2;
  htim2.Instance = TIM2;

  stepper.htim = &htim2;
  stepper.gpios = gpios;
  stepper.period = 7999U;
  stepper.presc = 9U;
  Stepper_Init(&stepper);

  Stepper_InitStruct_t stepper2; 
  stepper2.htim = &htim3;
  stepper2.gpios = gpios;
  stepper2.period = 7999U;
  stepper2.presc = 99U;
  Stepper_Init(&stepper2);

  Stepper_Step(&stepper2, 512, STEPPER_DIRECTION_FORWARD, mode_);

  HAL_Delay(500);
  Stepper_Halt(&stepper2);
  HAL_Delay(500);
  Stepper_Resume(&stepper2);

  STEPPER_POLL_FOR_FINISH(stepper2);
  HAL_Delay(500);
  Stepper_Step(&stepper2, 512, STEPPER_DIRECTION_REVERSE, mode_);
  
  while (1) ;
}