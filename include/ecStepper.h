/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 08-10-2026
Language/ver     : C++ in VS Code

Description      : ecStepper.h
/----------------------------------------------------------------*/

#ifndef EC_STEPPER_H
#define EC_STEPPER_H

#include "stm32f411xe.h"
#include "ecPinNames.h"
#include "ecRCC2.h"
#include "ecGPIO2.h"
#include "ecSysTick2.h"

#ifdef __cplusplus
extern "C" {
#endif

// Stepper Mode
#define HALF 0
#define FULL 1	 
#define DIR_CW 0
#define DIR_CCW 1	  


// 28BYJ-48 : 64 steps/rev x 1/32 gear = 2048 Full step
#define STEP_PER_REV  (64*32)		

// 28BYJ-48 : 128 steps/rev x 1/32 gear = 4096 Half step
// #define STEP_PER_REV  (128*32)		

//State number 
typedef enum {
	S0, S1, S2, S3, S4, S5, S6, S7
} StateNum;

// FULL stepping sequence  - FSM
typedef struct {	
  uint32_t next[2];
  uint8_t out[4];
} StateStepper_t;

// Initialization of Stepper Motor
void stepper_init(uint32_t mode, PinName_t *pinStepper);

// Converts Motor [rpm] to step delay in [usec]
void stepper_speed (uint32_t speedRPM);

// Run Stepper Motor for given steps and direction
void stepper_step(uint32_t steps, uint32_t direction);

// Stepper Motor Output for given state
void stepper_out(uint32_t state);

// Stop Stepper Motor
void stepper_stop(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // EC_STEPPER_H