/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Student ID       : 22200818
Created          : 10-22-2026
Language/ver     : C in VS Code

Description      : EC Test 1 (2026) Programming Test
                   
/----------------------------------------------------------------*/


#include "stm32f411xe.h"


//==================================================================
//  EC Library Include
//==================================================================
#include "ecSTM32F4v2.h"
#include "ecUART2.h"
#include "ecStepper.h"



//==================================================================
// User GPIO Pin definitions
//==================================================================

#define SW1 PD_2
#define SW2 PA_4
#define Photo PA_0
#define LED_PIN PB_12

#define MOTOR_REV 2
#define RPM 10

// Stepper ( A, B, AN, BN )
PinName_t pinStepper[4] = {PC_5, PC_6, PC_7, PC_8};

// FND segments
PinName_t pinFND[8]    = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

// FND digit select : FND_0 ~ FND_3
PinName_t selectFND[4]={PA_10, PA_11, PC_4, PC_3 };
uint8_t numDisplay = 0;
// Global Variables
// use   volatile uint32_t   instead of uint32_t if used in Interrupt ISR

//swtitch pressd flag
// blink

static volatile uint8_t command = 0;






//==================================================================
//  Setup 
//==================================================================
//motor mode
//motor mode output
//


void setup(void){
	RCC_PLL_init();					// System Clock = 84MHz
	SysTick_init();					// SysTick, 1 msec
	
	// USART2, 9600 baud
    USART2_init();
	USART2_baud(BAUD_9600);

	// GPIO Inputs
	
    GPIO_init(PB_12, OUTPUT);
  

    GPIO_init(Photo, INPUT);
    GPIO_pupd(Photo, PULL_UP);


	// EXTI 
	GPIO_init(SW2, INPUT);
    GPIO_pupd(SW2, PULL_UP);
	GPIO_init(SW1, INPUT);
    GPIO_pupd(SW1, PULL_UP);

    // External Interrupt : Falling Edge
    EXTI_init(SW2, FALL, 1000);
	EXTI_init(SW1, FALL, 1000);

	FND_init(pinFND);
    FND_select_init(selectFND);   
    

	// GPIO Outputs	
	 GPIO_init(LED_PIN, OUTPUT);

	

    // Stepper Motor
    stepper_init(FULL, pinStepper);
    //stepper_speed(2);
    //stepper_speed(6);
    stepper_speed(RPM);

   
    delay_us_init();

}


//==================================================================
//  Main
//==================================================================
int main(void){

		// 2sec tick for LED
	uint8_t numDisplay = 0;
    // Display number on FND0
 
	uint32_t steps=STEP_PER_REV* MOTOR_REV;

	int buttonState = 0;

	setup();

	printf("MODE0\r\n");

	while(1){
    	buttonState = GPIO_read(Photo);
         // Check received command
        if(command != 0){
            uint8_t ch = command;
            command = 0;

            if(ch == 'C' || ch == 'c'){
				numDisplay=1;
				printf("MODE1\r\n");
				GPIO_write(LED_PIN, HIGH);
                stepper_step(steps, DIR_CW);
				
            
				
            }
            else if(ch == 'B' || ch == 'b'){
				numDisplay=1;
				printf("MODE1\r\n");
				GPIO_write(LED_PIN, HIGH);
                stepper_step(steps, DIR_CCW);
				
                
				
            }
			else if(buttonState == HIGH){
       
			GPIO_write(LED_PIN, LOW);
			stepper_stop();
            printf("Motor STOP\r\n");
			}

        }
		
		FND_select(0);
        FND_display(numDisplay);
        // Button pressed
        if(GPIO_read(SW1) == LOW){

            numDisplay++;

           
            if(numDisplay > 3)
                numDisplay = 3;	
		// Wait until button is released
            while(GPIO_read(SW1) == LOW);
        }
		else if(GPIO_read(SW2) == LOW){

			numDisplay--;

            if(numDisplay < 1)
                numDisplay = 0;

		// Wait until button is released
            while(GPIO_read(SW2) == LOW);
		}    
		
     
       
	}

    

		// Check PAUSE/RESUME 
		//motor_pause = !(GPIO_read());
		if(motor_pause){ 
		led_blink=1;
		sw_pressed=0;
		stepper_stop;
		}
		else{
			led_blink=0; GPIO_write(LED0,HIGH);
			if(sw_pressed){
				nextState();
				sw_pressed=0;
			}
			stateOutput();
			

	}
		// Related to controlling motor speed/dir etc
		// HINT:  use  stepper_speed(vel);	only when vel>0
		// stepper_step(1, motor_dir);	 // Keep step=1 or a low value to stop blocking other code
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE


		// Print to the serial monitor every 2 sec
		// PAUSE, blink at 2 sec
		//     e.g.  printf("Mode=%lu Speed=%lu DIR=%lu\r\n", ... );
		// HINT:  msTicks in ecSysTick2.h  is increasing at a 1 msec rate accurately.
		// You can use msTicks  to count every 2000 msec 
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE

			
	delay_ms(1);  // use this delay
	
}


//==================================================================
//  EXTI Interrupt Service Routines
//==================================================================

	//sw(pd_2): speed-up s0>s1>s2>s3>s3...
	void EXTI2_IRQHandler(void){
		if(is_pending_EXTI(SW1)){
			)
		}
	}
//

// sw2 pa4 speed down
void EXTI2_IRQHandler(void){
		if(is_pending_EXTI(SW1)){
			)
		}
	}

//==================================================================
//  UART2 Interrupt Service Routines
//==================================================================
void USART2_IRQHandler(void){
	if(is_USART2_RXNE()){
	
	// HINT: 	comparing a character:  
	//  		uint8_t ch = USART2_read();		
	//			if(ch == 'C' || ch == 'c')	 etc..
	//
	 uint8_t data = USART2_read();

        if(data == 'C' || data == 'c' ||
           data == 'B' || data == 'b'){
            command = data;
        }
    }

}
