# STM32_Stepper

This is a fairly simple library to control (bipolar) stepper motors using STM32F1 and ULN2003APG chip or something likewise (for example discrete transistors in the same configuration as in the afformentioned chip). It is written using HAL, so it is required. It will probably work with other STM32 MCUs, but it will be necessary to change includes in main.h.

## Disclaimer

This library is very sketchy, likely buggy and inefficient. So DO NOT use it in any critical applications, where human lives, health or property are at stake. This library was written strictlyin educational purposes and author denies any responsibility for any damage taken by usage of it.

## How to use it

### Peripherals configuration

This library utilizes basic timers and GPIO pins. It is necessary to initialize timer in basic interrupt mode and add `Stepper_SingleStep(TIM_HandleTypeDef *htim)` function call in callback or interrupt handler. Do not start this timer anywhere else in code!
> This function is relatively large, so it is possible that it will cause reduced system responsiveness and missed interrupts on high update frequencies, so it is advised to proceed with caution. 
It is also necessary to configure 4 GPIO pins for motor coils control. Currently, pins must be in the same port (e. g. GPIOA).

### Software configuration

To initialize stepper motor it is necessary to create `Stepper_InitStruct_t` structure. It's fields are:

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

Here:
 - `gpios` is an array of 4, that contains control GPIO pins;
 - `htim`  is a pointer to timer structure;
 - `period` is a timer update period;
 - `presc` is a timer prescaler value.
 
 Other fields are used in internal functions and it is not advised to interacti with them directly.

 Then it is needed to call `Steper_Init(Stepper_InitStruct_t *stepper)` function with this structure.

 ### Running steppers

 To run the stepper call `Stepper_Step(Stepper_InitStruct_t *stepper, int steps, StepperDirec direc, StepperModes mode)` function. It's arguments are:

  - `stepper` is a pointer to `Stepper_InitStruct_t` structure;
  - `direc` is the direction of the stepper (`STEPPER_DIREC_FORWARD` or `STEPPER_DIREC_REVERSE`);
  - `mode` is the mode of the stepper (`STEPPER_MODE_WAVE`, `STEPPER_MODE_STEP`, `STEPPER_MODE_HALFSTEP`).

It will start timer in basic interrupt mode. Steps will happen when timer overflow event causes interrupt.

It is possible to pause and resume stepper using `Stepper_Pause` and `Steper_Resume` functions. They just stop the timer. `Stepper_Pause` `hold` argument defines, whether motor coils will be energized during pause (one of `STEPPER_STATE_HOLDING` or `STEPPER_STATE_FREE`). It is also possible to halt the stepper with `Stepper_Halt`. It stops the timer and states stepper state to `STEPPER_STATE_READY`.

>Do not call `Stepper_SingleStep` anywhere else but callback/ISR, unless you know, what you're doing. But then you probaly can write a better library :).

`Stepper_PollForFinish` is just a busy waiting loop, that is waiting for `Stepper->__state` to become `STEPPER_STATE_READY`. It happens, when stepper finished running, but also in some other cases.

### Types and getters/setters

There are some service types defined in this library to increase readability:

 - `StepperModes`: stepper run mode, one of `STEPPER_MODE_WAVE`, `STEPPER_MODE_STEP` or `STEPPER_MODE_HALFSTEP`;
 - `StepperDirec`: stepper rotation direction, one of `STEPPER_DIREC_FORWARD` or `STEPPER_DIREC_REVERSE`;
 - `Stepper_State`: current stepper state in library, one of:
    - `STEPPER_STATE_READY`: stepper is configured and ready to run, but is not running;
    - `STEPPER_STATE_RUNNING`: stepper is currently runnung;
    - `STEPPER_STATE_HOLDING`: stepper is currently paused and it's coils are energized;
    - `STEPPER_STATE_FREE`: stepper is currently paused and it's coils are  not energized.
 - `ConfigState`: stepper function call status. Is one of the following:
    - `STEPPER_OK`: everything is (probably) fine;
    - `STEPPER_ERROR_CONFIG`: there is some error on the configuration step, possibly a NULL value;
    - `STEPPER_ERROR_CONTROL`: there is some error during control step, possibly a NULL or otherwise invalid value;
    - `STEPPER_ERROR_HAL`: there is some error during control step that happened in HAL functions, possibly some peripherals are not configured properly or there is some other mistake on user (or my) side.
    - `STEPPER_ERROR_RUNTIME`: currently in developement.

Stepper state, direction etc. can be read or written using an appropriate getter/setter (`Stepper_Get(Set)<Field>`).

>It is not advised to use `Stepper_SetState` function, unless you know, what you're doing. But then again, if you are, then you probably don't need all this.

