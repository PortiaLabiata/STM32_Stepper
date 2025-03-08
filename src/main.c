#include "main.h"
#define mode_ STEPPER_MODE_HALFSTEP

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  Stepper_InitStruct_t stepper;
  uint16_t gpios[4] = {GPIO_PIN_0,GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3}; 
  stepper.gpios = gpios;
  stepper.period = 7999U;
  stepper.presc = 9U;
  Stepper_Init(&stepper);

  Stepper_Step(&stepper, 512, STEPPER_DIRECTION_FORWARD, mode_);

  HAL_Delay(500);
  Stepper_Halt(&stepper);
  HAL_Delay(500);
  Stepper_Resume(&stepper);

  STEPPER_POLL_FOR_FINISH();
  HAL_Delay(500);
  Stepper_Step(&stepper, 512, STEPPER_DIRECTION_REVERSE, mode_);
  
  while (1) ;
}