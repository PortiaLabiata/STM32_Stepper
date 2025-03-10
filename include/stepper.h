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

/* Macroses */

#define STEPPER_POLL_FOR_FINISH(__STEPPER__) \
 while (Stepper_GetState(__STEPPER__) == STEPPER_STATE_RUNNING) ;

/* Magic numbers */

#define GPIO_MASK 0b1111L;
#define RUN_INDEFINITELY -1
#define MAX_STEPPERS 4

typedef enum {
    STEPPER_MODE_WAVE,
    STEPPER_MODE_STEP,
    STEPPER_MODE_HALFSTEP
} StepperModes;

typedef enum {
    STEPPER_DIRECTION_FORWARD,
    STEPPER_DIRECTION_REVERSE
} StepperDirec;

typedef enum {
    STEPPER_STATE_READY,
    STEPPER_STATE_RUNNING,
    STEPPER_STATE_HOLDING,
    STEPPER_STATE_FREE
} Stepper_State;

typedef enum {
    STEPPER_OK,
    STEPPER_ERROR_CONTROL,
    STEPPER_ERROR_CONFIG,
    STEPPER_INVALID_MODE,
    STEPPER_INVALID_DIREC,
    STEPPER_ERROR_RUNTIME // Может потом придумаю, что а этим делать, пока добавлю
} ConfigState;


typedef struct {
    uint16_t *gpios;
    TIM_HandleTypeDef *htim;
    uint16_t period;
    uint32_t presc;
    Stepper_State __state;
    circular_buffer_t __buffer;
    StepperModes __mode;
    StepperDirec __direc;
    uint32_t __steps_left;
} Stepper_InitStruct_t;

/* Global variables */

/* Control functions */

ConfigState Stepper_Init(Stepper_InitStruct_t *stepper);
ConfigState  Stepper_Step(Stepper_InitStruct_t *stepper, int steps, StepperDirec direc, StepperModes mode);
ConfigState Stepper_Pause(Stepper_InitStruct_t *stepper, Stepper_State hold);
ConfigState Stepper_Halt(Stepper_InitStruct_t *stepper);
void Stepper_Resume(Stepper_InitStruct_t *stepper);
ConfigState Stepper_PollForFinish(Stepper_InitStruct_t *stepper);
ConfigState Stepper_SingleStep(TIM_HandleTypeDef *htim);

/* Getters and setters */

StepperModes Stepper_GetMode(Stepper_InitStruct_t stepper);
StepperDirec Stepper_GetDirec(Stepper_InitStruct_t stepper);
Stepper_State Stepper_GetState(Stepper_InitStruct_t stepper);
ConfigState Stepper_SetState(Stepper_InitStruct_t *stepper, Stepper_State state);

#endif