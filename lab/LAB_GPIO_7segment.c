/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 23-09-2026 
Language/ver     : C++ in VS Code

Description      : LAB_GPIO_7segment
/----------------------------------------------------------------*/

#include "stm32f4xx.h"
#include "ecGPIO2.h"
#include "ecRCC2.h"

#define BUTTON_PIN PA_4

PinName_t pinFND[8]    = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
PinName_t selectFND[4]={PA_10, PA_11, PC_4, PC_3 };


void setup(void){

    // Intialize System Clock
    RCC_HSI_init();    

    // Intialize FND pins and Others
    FND_init(pinFND);
    FND_select_init(selectFND);   
    
     // Button: Input, Pull-up
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PULL_UP);
  
}

int main(void) {

    setup();

    uint8_t numDisplay = 0;

    while (1) {

      // Display number on FND0
        FND_select(0);
        FND_display(numDisplay);

        // Button pressed
        if(GPIO_read(BUTTON_PIN) == LOW){

            numDisplay++;

            // 9 -> 0
            if(numDisplay > 9)
                numDisplay = 0;

            // Wait until button is released
            while(GPIO_read(BUTTON_PIN) == LOW);
        }
    }
}

