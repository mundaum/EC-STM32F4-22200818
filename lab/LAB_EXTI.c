/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 05-03-2021
Modified         : 07-10-2026
Language/ver     : C++ in VS Code

Description      : LAB_EXTI.c
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

//#include "ecRCC2.h"
//#include "ecGPIO2.h"
//#include "ecSysTick2.h"
//#include "ecEXTI2.h"

#define SW1_PIN  PD_2
#define SW2_PIN  PA_4
#define FND0_PIN PA_10


// Counter value
int count = 0;

// 0 : Counting
// 1 : Pause
int pauseFlag = 0;


// 7-Segment pins
// a, b, c, d, e, f, g, dot
PinName_t fndPins[8] = {
    PB_0, PB_1, PB_2, PB_3,
    PB_4, PB_5, PB_6, PB_7
};


// Initialization
void setup(void)
{
    // Default MCU Initialization
    // PLL 84MHz, SysTick 1ms
    MCU_init();


    // ==================================================
    // SW1 : PD_2
    // Digital Input, Pull-Up
    // ==================================================
    GPIO_init(SW1_PIN, INPUT);
    GPIO_pupd(SW1_PIN, PULL_UP);

    // External Interrupt : Falling Edge
    EXTI_init(SW1_PIN, FALL, 10);


    // ==================================================
    // SW2 : PA_4
    // Digital Input, Pull-Up
    // ==================================================
    GPIO_init(SW2_PIN, INPUT);
    GPIO_pupd(SW2_PIN, PULL_UP);

    // External Interrupt : Falling Edge
    EXTI_init(SW2_PIN, FALL, 10);


    // ==================================================
    // 7-Segment DOUT : PB_0 ~ PB_7
    // Output, Push-Pull, No PUPD, Medium Speed
    // ==================================================
    FND_init(fndPins);

    for(int i = 0; i < 8; i++)
    {
        GPIO_otype(fndPins[i], PUSH_PULL);
        GPIO_pupd(fndPins[i], NO_PUPD);
        GPIO_ospeed(fndPins[i], MID_SPEED);
    }


    // ==================================================
    // FND0 Selection : PA_10
    // Digital Output, Push-Pull
    // ==================================================
    GPIO_init(FND0_PIN, OUTPUT);
    GPIO_otype(FND0_PIN, PUSH_PULL);

    // Select FND0
    GPIO_write(FND0_PIN, HIGH);


    // Initial display
    FND_display(count);
}


// MAIN ------------------------------------------------
int main(void)
{
    setup();

    while(1)
    {
        // 1 second interval
        delay_ms(1000);

        // Count only when SW2 is not paused
        if(pauseFlag == 0)
        {
            count++;

            // Repeat 0 ~ 9
            if(count > 9)
            {
                count = 0;
            }
        }

        // Display current counter value
        FND_display(count);
    }
}



// =====================================================
// SW1 : PD_2 -> EXTI2
// Reset counter to zero
// =====================================================
void EXTI2_IRQHandler(void)
{
    if(is_pending_EXTI(SW1_PIN))
    {
        count = 0;

        // Immediately display zero
        FND_display(count);

        clear_pending_EXTI(SW1_PIN);
    }
}



// =====================================================
// SW2 : PA_4 -> EXTI4
// Pause / Resume counting
// =====================================================
void EXTI4_IRQHandler(void)
{
    if(is_pending_EXTI(SW2_PIN))
    {
        // Toggle Pause / Resume
        pauseFlag ^= 1;

        clear_pending_EXTI(SW2_PIN);
    }
}