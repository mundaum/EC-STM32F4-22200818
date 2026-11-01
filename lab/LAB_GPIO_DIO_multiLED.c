/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 14-09-2026 
Language/ver     : C++ in VS Code

Description      : LAB_GPIO_DIO_multiLED
/----------------------------------------------------------------*/

#include "ecRCC2.h"
#include "ecGPIO2.h"

#define LED0_PIN PB_12
#define LED1_PIN PB_13
#define LED2_PIN PB_14
#define LED3_PIN PB_15

#define BUTTON_PIN PA_4

//Initialize
void setup(void) {
    RCC_HSI_init();
    // Button: PA4, Input, Pull-up
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PULL_UP);

    // LED0
    GPIO_init(LED0_PIN, OUTPUT);
    GPIO_otype(LED0_PIN, OPEN_DRAIN);
    GPIO_pupd(LED0_PIN, PULL_UP);
    GPIO_ospeed(LED0_PIN, MID_SPEED);

    // LED1
    GPIO_init(LED1_PIN, OUTPUT);
    GPIO_otype(LED1_PIN, OPEN_DRAIN);
    GPIO_pupd(LED1_PIN, PULL_UP);
    GPIO_ospeed(LED1_PIN, MID_SPEED);

    // LED2
    GPIO_init(LED2_PIN, OUTPUT);
    GPIO_otype(LED2_PIN, OPEN_DRAIN);
    GPIO_pupd(LED2_PIN, PULL_UP);
    GPIO_ospeed(LED2_PIN, MID_SPEED);

    // LED3
    GPIO_init(LED3_PIN, OUTPUT);
    GPIO_otype(LED3_PIN, OPEN_DRAIN);
    GPIO_pupd(LED3_PIN, PULL_UP);
    GPIO_ospeed(LED3_PIN, MID_SPEED);
}
int main(void) {
    setup();
    int buttonState = 0;
    int ledIndex = 0;

    // Initial state LED0 ON
    GPIO_write(LED0_PIN, HIGH);
    GPIO_write(LED1_PIN, LOW);
    GPIO_write(LED2_PIN, LOW);
    GPIO_write(LED3_PIN, LOW);


    while (1) {

        // Read button state
        buttonState = GPIO_read(BUTTON_PIN);

        // Pull-up button
        if (buttonState == LOW) {

            // Move to next
            ledIndex++;

            // LED3 -> LED0
            if (ledIndex > 3)
                ledIndex = 0;

            // Turn all LEDs OFF
            GPIO_write(LED0_PIN, LOW);
            GPIO_write(LED1_PIN, LOW);
            GPIO_write(LED2_PIN, LOW);
            GPIO_write(LED3_PIN, LOW);

            // Turn ON selected LED
            if (ledIndex == 0)
                GPIO_write(LED0_PIN, HIGH);

            else if (ledIndex == 1)
                GPIO_write(LED1_PIN, HIGH);

            else if (ledIndex == 2)
                GPIO_write(LED2_PIN, HIGH);

            else if (ledIndex == 3)
                GPIO_write(LED3_PIN, HIGH);

            
                while (GPIO_read(BUTTON_PIN) == LOW);
                // Wait for button to move only one step 
        }
    }
}

