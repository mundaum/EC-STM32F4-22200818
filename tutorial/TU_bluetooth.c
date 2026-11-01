#include "ecSTM32F4v2.h"

uint8_t BT_string[] = "Hi_BT\r\n";

void setup(void){
    RCC_PLL_init();
    SysTick_init();

    USART2_init();
    USART1_init();
}

int main(void){
    setup();

    printf("MCU Initialized\r\n");

    while(1){
        USART1_write(BT_string, 7);
        printf("USART1 TX: Hi_BT\r\n");
        delay_ms(2000);
    }
}