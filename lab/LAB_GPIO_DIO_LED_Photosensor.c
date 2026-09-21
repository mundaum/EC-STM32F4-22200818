/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 14-09-2026 
Language/ver     : C++ in VS Code

Description      : LAB_GPIO_DIO_LED_Photosensor
/----------------------------------------------------------------*/

#include "ecRCC2.h"
#include "ecGPIO2.h"

#define LED_PIN PB_12
#define BUTTON_PIN PA_0

//Initialiization
void setup(void) {
    RCC_HSI_init();
    // intialize the push button pin as input
    GPIO_init(BUTTON_PIN, INPUT);
    // intialize the LED pin as output
    GPIO_init(LED_PIN, OUTPUT);
    // LED PB_12  Pull-UP,Open-Drain, Medium Speed
    GPIO_ospeed(LED_PIN, MID_SPEED);
    GPIO_otype(LED_PIN, OPEN_DRAIN);
    GPIO_pupd(LED_PIN, PULL_UP);
}

int main(void) {
    setup();
    int buttonState = 0;

    while (1) {
        //check if the pushbutton is pressed. turn LED on/off accordinglt:
        buttonState = GPIO_read(BUTTON_PIN);
        if(buttonState) GPIO_write(LED_PIN, LOW);
        else GPIO_write(LED_PIN, HIGH);
        delay_ms_HSI(100); // delay 100  ms
    }
}