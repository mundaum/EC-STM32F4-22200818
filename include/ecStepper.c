/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 08-10-2026
Language/ver     : C++ in VS Code

Description      : ecStepper.c
/----------------------------------------------------------------*/

#include "stm32f4xx.h"
#include "ecStepper.h"


// Stepper Motor variable
static PinName_t stepperPins[4];
static volatile uint32_t _steps;
static volatile uint32_t _step_delay = 100;		
static const StateStepper_t *fsm;				// points at fsm_full or fsm_half (chosen in stepper_init)
static volatile uint32_t state = 0;				// current FSM state



// Output: A B A' B'
static const StateStepper_t fsm_Full[4] = {	
 {{S1,S3},{HIGH,HIGH,LOW,LOW}},	
 {{S2,S0},{LOW,HIGH,HIGH,LOW}},
 {{S3,S1},{LOW,LOW,HIGH,HIGH}},
 {{S0,S2},{HIGH,LOW,LOW,HIGH}}
	
};

// Output: A B A' B'
static const StateStepper_t fsm_Half[8] = {  
	{{S1,S7},{1,0,0,0}},
	{{S2,S0},{1,1,0,0}},
	{{S3,S1},{0,1,0,0}},
	{{S4,S2},{0,1,1,0}},
	{{S5,S3},{0,0,1,0}},
	{{S6,S4},{0,0,1,1}},
	{{S7,S5},{0,0,0,1}},
	{{S0,S6},{1,0,0,1}}

	
};


// Initialization of Stepper Motor
void stepper_init(uint32_t mode, PinName_t *pinStepper){
	if (mode == HALF) 
		fsm = fsm_Half;
	else
		fsm = fsm_Full;


 // GPIO Digital Out Initialization
 
 // Pin Assignment
    stepperPins[0] = pinStepper[0];  // A
    stepperPins[1] = pinStepper[1];  // B
    stepperPins[2] = pinStepper[2];  // AN
    stepperPins[3] = pinStepper[3];  // BN

    // GPIO Output Initialization
    GPIO_init(stepperPins[0], OUTPUT);
    GPIO_init(stepperPins[1], OUTPUT);
    GPIO_init(stepperPins[2], OUTPUT);
    GPIO_init(stepperPins[3], OUTPUT);

    // GPIO Output Speed
    GPIO_ospeed(stepperPins[0], FAST_SPEED);
    GPIO_ospeed(stepperPins[1], FAST_SPEED);
    GPIO_ospeed(stepperPins[2], FAST_SPEED);
    GPIO_ospeed(stepperPins[3], FAST_SPEED);
}

// Converts Motor [rpm] to step delay in [usec]
void stepper_speed(uint32_t speedRPM){
    _step_delay = 60*1000/(STEP_PER_REV*speedRPM);  // in [msec]
}


// Run Stepper Motor for given steps and direction
void stepper_step(uint32_t steps, uint32_t direction){
	// Exit if stepper_init() not called 
	if (fsm == 0) return;
	_steps = steps;

	// run for step size: 
	for (uint32_t i = 0; i < _steps; i++){
		// Update Present State
		if (direction == DIR_CW)
			state = fsm[state].next[DIR_CW];
		else
			state = fsm[state].next[DIR_CCW];
		
		// Output of Present State
		stepper_out(state);
	
		// Delay for steppermotor speed us
		delay_ms(_step_delay);
	}
}

// Stepper Motor Output for given state
void stepper_out(uint32_t state){
	// stepperPins[0]=A, stepperPins[1]=B, ...

	// out[0]=A, out[1]=B, ...
	for (int i = 0; i < 4; i++) {
		GPIO_write(stepperPins[i], fsm[state].out[i]);
	}

}


// Stop Stepper Motor
void stepper_stop(void) {
    _steps = 0;    
    
	// All pins DigitalOut '0'
	for (int i = 0; i < 4; i++) {
		GPIO_write(stepperPins[i], LOW);
	}
}



