#include "main.h"
#define mode_ STEPPER_MODE_HALFSTEP

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  Stepper_Init(11999U, 9U);

  Stepper_Step(256, STEPPER_DIRECTION_REVERSE, mode_);
  POLL_FOR_READINESS();
  Stepper_Step(256, STEPPER_DIRECTION_FORWARD, mode_);
  POLL_FOR_READINESS();
  Stepper_Step(256, STEPPER_DIRECTION_REVERSE, mode_);
  POLL_FOR_READINESS();
  Stepper_Step(256, STEPPER_DIRECTION_FORWARD, mode_);
  
  while (1) ;
}