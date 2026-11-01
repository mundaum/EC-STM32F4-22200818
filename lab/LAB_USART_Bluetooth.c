/*----------------------------------------------------------------
@ Embedded Controller - Handong Global University
Author           : Daum Moon
Created          : 10-20-2026

Description      : LAB_USART_Bluetooth.c
----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"
#include "ecUART2.h"
#include "ecStepper.h"
#include <string.h>

// Stepper Motor Pins (A, B, AN, BN)
PinName_t pinStepper[4] = {PB_10, PB_4, PB_5, PB_3};

#define RPM 10
#define MOTOR_REV 2

static volatile uint8_t command = 0;

// Send message through Bluetooth
void BT_send(const char *message){
    USART1_write((uint8_t*)message, strlen(message));
}

void setup(void){
    RCC_PLL_init();
    SysTick_init();
    delay_us_init();

    // USART1: HC-06 Bluetooth
    USART1_init();
    USART1_baud(BAUD_9600);

    // LD2 LED
    GPIO_init(PA_5, OUTPUT);
    GPIO_write(PA_5, LOW);

    // Stepper Motor
    stepper_init(FULL, pinStepper);
    stepper_speed(RPM);
}

int main(void){
    setup();

    uint32_t steps = STEP_PER_REV * MOTOR_REV;
    uint32_t remainingSteps = 0;
    uint32_t direction = DIR_CW;

    BT_send("MCU Initialized\r\n");

    while(1){

        // Check received command
        if(command != 0){
            uint8_t key = command;
            command = 0;

            if(key == 'H'){
                GPIO_write(PA_5, HIGH);
                BT_send("LED ON\r\n");
            }

            else if(key == 'L'){
                GPIO_write(PA_5, LOW);
                BT_send("LED OFF\r\n");
            }

            else if(key == 'J'){
                direction = DIR_CW;
                remainingSteps = steps;
                BT_send("CW Start\r\n");
            }

            else if(key == 'K'){
                direction = DIR_CCW;
                remainingSteps = steps;
                BT_send("CCW Start\r\n");
            }

            else if(key == 'P'){
                remainingSteps = 0;
                stepper_stop();
                BT_send("Motor STOP\r\n");
            }
        }

        // Run the motor one step at a time
        if(remainingSteps > 0){
            stepper_step(1, direction);
            remainingSteps--;

            if(remainingSteps == 0){
                stepper_stop();
                BT_send("Motor Finished\r\n");
            }
        }
    }
}

// USART1: Receive command from Bluetooth
void USART1_IRQHandler(void){
    if(is_USART1_RXNE()){
        uint8_t data = USART1_read();

        if(data == 'H' || data == 'L' ||
           data == 'J' || data == 'K' ||
           data == 'P'){
            command = data;
        }
    }
}

