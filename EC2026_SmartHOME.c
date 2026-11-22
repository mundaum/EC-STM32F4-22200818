/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : Daum Moon
Created          : 12-11-2026
Language/ver     : C++ in VS Code

Description      : smart_home.c
/----------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ecSTM32F4v2.h"


//==================================================
// Pins: fixed by project requirements
//==================================================
#define MODE_BUTTON_PIN PA_10   // EXTI10 / Key-C / D2
#define DHT11_PIN       PA_6
#define FAN_PIN         PB_7    // TIM4 CH2, 1 kHz PWM
#define WINDOW_PIN      PC_7    // TIM3 CH2, 50 Hz PWM

#define RGB_RED_PIN     PB_13
#define RGB_GREEN_PIN   PB_14
#define RGB_BLUE_PIN    PB_15
#define LIGHT_PIN       PA_0    // ADC1 JDR1
#define POT_PIN         PC_1    // ADC1 JDR2
#define PIR_PIN         PB_3    // EXTI3


//==================================================
// Ventilation thresholds: adjust to test the real DHT11
//==================================================
#define TEMP_HIGH       30
#define TEMP_LOW        24

#define HUM_HIGH        35
#define HUM_LOW         23

#define FAN_SPEED       0.7f
// These are the original uploaded values; calibrate for physical installation.
#define WINDOW_OPEN     1400
#define WINDOW_CLOSE    860

//==================================================
// 250-ms TIM1 ticks
//==================================================
#define AWAY_EXIT_TICKS   20U   // 5 seconds
#define YELLOW_LED_TICKS  12U   // 3 seconds
#define PIR_HOME_COOLDOWN_TICKS 80U  // 20 seconds between accepted HOME PIR events
#define GREEN_HALF_TICKS   4U   // on 1 s, off 1 s
#define RED_HALF_TICKS     2U   // on 0.5 s, off 0.5 s
#define LCD_BLINK_TICKS    4U   // 1 s

// RGB colors; use names that cannot conflict with library macros
#define LED_OFF         0U
#define LED_RED         1U
#define LED_GREEN       2U
#define LED_WHITE       3U
#define LED_YELLOW      4U

//==================================================
// System states
//==================================================
typedef enum {
    HOME = 0,
    AWAY = 1,
    ALARM = 2
} SystemMode;

// Read from an interrupt handler, so the current state is volatile.
volatile SystemMode mode = HOME;

//==================================================
// Global variables
//==================================================
volatile uint8_t button_event = 0;
volatile uint8_t pir_event = 0;
volatile uint8_t alarm_clear_event = 0; // USART6 (or USB fallback) 'x' / 'X'
volatile uint8_t bt_rx_event = 0;
volatile uint8_t bt_rx_char = 0;
volatile uint32_t ticks = 0;              // TIM1: 250 ms
uint32_t last_button_time = 0;

volatile uint32_t away_start = 0;         // read from PIR IRQ
uint8_t away_countdown = 0;
uint32_t alarm_start = 0;

// Real DHT11 measurements
int temperature = 0;
int humidity = 0;
uint8_t dht_valid = 0;
uint8_t first_dht_read = 1;
uint8_t ventilation_on = 0;
uint32_t last_dht_time = 0;
uint32_t dht_fail_count = 0;

// LCD
uint8_t lcd_ready = 0;

// ADC: updated by JADC interrupt
volatile uint32_t light_val = 0;
volatile uint32_t poten_val = 0;
volatile uint8_t adc_valid = 0;

// PIR in HOME: 3-second yellow; cooldown limits repeated activations.
uint8_t yellow_active = 0;
uint32_t yellow_start = 0;
uint32_t pir_count = 0;
uint8_t home_pir_first = 1;
uint32_t last_home_pir_tick = 0;

//==================================================
// Function prototypes
//==================================================
void setup(void);
void mode_changed(void);
void home_tasks(void);
void away_tasks(void);
void handle_pir_event(void);

void ventilation(void);
void ventilation_off(void);

void updateLCD(void);
void LCD_writeLine(uint8_t row, const char *message);

void LED_color(uint8_t color);
void Lighting(void);
void send_status(const char *message);

//==================================================
// Setup
//==================================================
void setup(void)
{
    RCC_PLL_init();              // 84 MHz
    SysTick_init();
    USART2_init();              // USB virtual COM: TX PA2 / RX PA3
    USART2_baud(BAUD_9600);     // Use 9600 baud in TeraTerm
    USART6_init();              // Bluetooth: PA11/PA12, AF8, 9600

    // Key-C: PA10 EXTI10.
    // IMPORTANT: if your confirmed working input used different settings,
    // keep those exact two lines from your tested button program.
    GPIO_init(MODE_BUTTON_PIN, INPUT);
    GPIO_pupd(MODE_BUTTON_PIN, PULL_DOWN);
    EXTI_init(MODE_BUTTON_PIN, RISE, 0);

    // PIR: PB3 EXTI3, rising edge
    GPIO_init(PIR_PIN, INPUT);
    EXTI_init(PIR_PIN, RISE, 1);

    // Control tick: TIM1 250 ms
    TIM_UI_init(TIM1, 250);

    // DHT11: PA6
    DHT11_init(DHT11_PIN);

    // Ventilation fan: PB7 TIM4 CH2, 1 ms period
    PWM_init(FAN_PIN, 1);
    PWM_duty(FAN_PIN, 0.0f);

    // Window servo: PC7 TIM3 CH2, 20 ms period
    PWM_init(WINDOW_PIN, 20);
    PWM_pulsewidth_us(WINDOW_PIN, WINDOW_CLOSE);

    // RGB: digital GPIO. Teammate's tested connection uses HIGH = ON.
    GPIO_init(RGB_RED_PIN, OUTPUT);
    GPIO_init(RGB_GREEN_PIN, OUTPUT);
    GPIO_init(RGB_BLUE_PIN, OUTPUT);
    LED_color(LED_OFF);

    // Injected ADC: TIM5 triggers sequence every 125 ms
    JADC_init(LIGHT_PIN);
    JADC_init(POT_PIN);
    PinName_t Jseq[2] = {LIGHT_PIN, POT_PIN};
    JADC_sequence(Jseq, 2);
    JADC_trigger_init(TIM5, 125, ADC_RISE);

    // LCD: I2C1 PB8(SCL), PB9(SDA)
    I2C1_init();
    if (I2C1_write(LCD_ADDR, LCD_BACKLIGHT) == 0)
    {
        LCD_init();
        lcd_ready = 1;
        printf("LCD CONNECTED\r\n");
    }
    else
    {
        printf("LCD NOT FOUND\r\n");
    }
}

//==================================================
// Button interrupt: only set a flag; no printf or motor control
//==================================================
void EXTI15_10_IRQHandler(void)
{
    if (is_pending_EXTI(MODE_BUTTON_PIN))
    {
        button_event = 1;
        EXTI->PR = (1UL << 10);
    }
}

//==================================================
// PIR interrupt: ignore intrusion during AWAY 5-second exit delay
//==================================================
void EXTI3_IRQHandler(void)
{
    if (is_pending_EXTI(PIR_PIN))
    {
        if (mode == HOME ||
            (mode == AWAY &&
             (uint32_t)(ticks - away_start) >= AWAY_EXIT_TICKS))
        {
            pir_event = 1;
        }

        // EXTI3 pending bit is W1C.
        EXTI->PR = (1UL << 3);
    }
}

//==================================================
// TIM1: 250-ms interrupt
//==================================================
void TIM1_UP_TIM10_IRQHandler(void)
{
    if (is_UIF(TIM1))
    {
        ticks++;
        clear_UIF(TIM1);
    }
}

//==================================================
// JADC interrupt: read JDR1 and JDR2
//==================================================
void ADC_IRQHandler(void)
{
    if (is_ADC_JEOC())
    {
        light_val = JADC_read(1);
        poten_val = JADC_read(2);
        adc_valid = 1;
        clear_ADC_JEOC();
    }
}

//==================================================
// Keep USB USART2 command input as a test fallback.
// Actual Bluetooth input is handled by USART6_IRQHandler below.
//==================================================
void USART2_IRQHandler(void)
{
    if (is_USART2_RXNE())
    {
        uint8_t received = USART2_read();
        if (received == 'x' || received == 'X')
        {
            alarm_clear_event = 1;
        }
    }
}

//==================================================
// Bluetooth module sends bytes into USART6 RX (PA12).
// Do not printf() or send messages from this interrupt handler.
//==================================================
void USART6_IRQHandler(void)
{
    if (is_USART6_RXNE())
    {
        uint8_t received = USART6_read();
        bt_rx_char = received;
        bt_rx_event = 1;
        if (received == 'x' || received == 'X')
        {
            alarm_clear_event = 1;
        }
    }
}

//==================================================
// Status is sent to Bluetooth USART6 and mirrored to USB USART2.
// printf() still goes only to USB USART2 (debugging).
//==================================================
void send_status(const char *message)
{
    USART2_write((uint8_t *)message, (uint32_t)strlen(message));
    USART6_write((uint8_t *)message, (uint32_t)strlen(message));
}

//==================================================
// Mode change side-effects
//==================================================
void mode_changed(void)
{
    pir_event = 0;
    yellow_active = 0;

    if (mode == HOME)
    {
        away_countdown = 0;
        home_pir_first = 1;
        send_status("MODE : HOME\r\n");
    }
    else if (mode == AWAY)
    {
        away_start = ticks;
        away_countdown = 1;
        ventilation_off();
        send_status("MODE : AWAY (5s exit delay)\r\n");
    }
    else if (mode == ALARM)
    {
        alarm_start = ticks;
        away_countdown = 0;
        ventilation_off();
        send_status("ALARM : Intruder detected!\r\n");
    }
}

//==================================================
// PIR event processing in main loop
//==================================================
void handle_pir_event(void)
{
    if (!pir_event)
        return;

    pir_event = 0;

    if (mode == HOME)
    {
        // Avoid repeatedly restarting yellow on closely spaced PIR triggers.
        // This does NOT reduce the PIR module's physical detection range.
        if (!home_pir_first &&
            (uint32_t)(ticks - last_home_pir_tick) < PIR_HOME_COOLDOWN_TICKS)
        {
            return;
        }

        home_pir_first = 0;
        last_home_pir_tick = ticks;
        yellow_active = 1;
        yellow_start = ticks;
        pir_count++;
        char event_message[52];
        snprintf(event_message, sizeof(event_message),
                 "PIR DETECTED (HOME), count=%lu\r\n",
                 (unsigned long)pir_count);
        send_status(event_message);
    }
    else if (mode == AWAY &&
             (uint32_t)(ticks - away_start) >= AWAY_EXIT_TICKS)
    {
        mode = ALARM;
        mode_changed();
    }
}

//==================================================
// HOME/AWAY main tasks
//==================================================
void home_tasks(void)
{
    ventilation();
}

void away_tasks(void)
{
    if (away_countdown &&
        (uint32_t)(ticks - away_start) >= AWAY_EXIT_TICKS)
    {
        away_countdown = 0;
        send_status("AWAY COUNTDOWN DONE - ARMED\r\n");
    }
}

//==================================================
// Ventilation control (existing, known-working code)
//==================================================
void ventilation_off(void)
{
    ventilation_on = 0;
    PWM_duty(FAN_PIN, 0.0f);
    PWM_pulsewidth_us(WINDOW_PIN, WINDOW_CLOSE);
}

void ventilation(void)
{
    if (!first_dht_read &&
        (uint32_t)(msTicks - last_dht_time) < DHT11_MIN_PERIOD_MS)
    {
        return;
    }

    first_dht_read = 0;
    int err = DHT11_read();
    last_dht_time = msTicks;

    if (err != DHT11_OK)
    {
        dht_fail_count++;
        printf("DHT11 ERROR = %d, count = %lu\r\n",
               err, (unsigned long)dht_fail_count);
        return;
    }

    temperature = DHT11_temperature();
    humidity = DHT11_humidity();
    dht_valid = 1;

    printf("T = %d C, RH = %d %%\r\n", temperature, humidity);

    if (!ventilation_on &&
        (temperature > TEMP_HIGH || humidity > HUM_HIGH))
    {
        ventilation_on = 1;
        PWM_duty(FAN_PIN, FAN_SPEED);
        PWM_pulsewidth_us(WINDOW_PIN, WINDOW_OPEN);
        send_status("VENTILATION ON\r\n");
    }
    else if (ventilation_on &&
             (temperature < TEMP_LOW && humidity < HUM_LOW))
    {
        ventilation_off();
        send_status("VENTILATION OFF\r\n");
    }
}

//==================================================
// RGB LED function, adapted from teammate code
//==================================================
void LED_color(uint8_t color)
{
    GPIO_write(RGB_RED_PIN, LOW);
    GPIO_write(RGB_GREEN_PIN, LOW);
    GPIO_write(RGB_BLUE_PIN, LOW);

    switch (color)
    {
        case LED_RED:
            GPIO_write(RGB_RED_PIN, HIGH);
            break;
        case LED_GREEN:
            GPIO_write(RGB_GREEN_PIN, HIGH);
            break;
        case LED_YELLOW:
            GPIO_write(RGB_RED_PIN, HIGH);
            GPIO_write(RGB_GREEN_PIN, HIGH);
            break;
        case LED_WHITE:
            GPIO_write(RGB_RED_PIN, HIGH);
            GPIO_write(RGB_GREEN_PIN, HIGH);
            GPIO_write(RGB_BLUE_PIN, HIGH);
            break;
        default:
            break;
    }
}

//==================================================
// LED priority follows teammate in HOME: VENT green > PIR yellow > LIGHT white
//==================================================
void Lighting(void)
{
    static uint32_t previous_tick = UINT32_MAX;
    static uint8_t previous_mode = 255U;
    static uint8_t previous_vent = 255U;
    static uint8_t previous_yellow = 255U;

    uint32_t now = ticks;

    // End HOME PIR yellow 3 seconds after the most recent detection.
    if (yellow_active &&
        (uint32_t)(now - yellow_start) >= YELLOW_LED_TICKS)
    {
        yellow_active = 0;
    }

    // LED changes at most once per 250-ms tick or changed condition.
    if (now == previous_tick &&
        (uint8_t)mode == previous_mode &&
        ventilation_on == previous_vent &&
        yellow_active == previous_yellow)
    {
        return;
    }

    previous_tick = now;
    previous_mode = (uint8_t)mode;
    previous_vent = ventilation_on;
    previous_yellow = yellow_active;

    if (mode == ALARM)
    {
        // 0.5 s ON / 0.5 s OFF
        if ((((now - alarm_start) / RED_HALF_TICKS) % 2U) == 0U)
            LED_color(LED_RED);
        else
            LED_color(LED_OFF);
    }
    else if (mode == AWAY)
    {
        // First 3 s yellow, then off. PIR monitoring begins at 5 s.
        if ((uint32_t)(now - away_start) < YELLOW_LED_TICKS)
            LED_color(LED_YELLOW);
        else
            LED_color(LED_OFF);
    }
    else // HOME
    {
        if (ventilation_on)
        {
            // 1 s ON / 1 s OFF, as in teammate's code
            if (((now / GREEN_HALF_TICKS) % 2U) == 0U)
                LED_color(LED_GREEN);
            else
                LED_color(LED_OFF);
        }
        else if (yellow_active)
        {
            LED_color(LED_YELLOW);
        }
        else if (adc_valid && (light_val > poten_val + 100U))
        {
            // Match the latest teammate code: light_val > poten_val
            LED_color(LED_WHITE);
        }
        else
        {
            LED_color(LED_OFF);
        }
    }
}

//==================================================
// LCD: print a padded 16-character line only if it changed
//==================================================
void LCD_writeLine(uint8_t row, const char *message)
{
    static char previous[2][17] = {"", ""};
    char line[17];

    if (row > 1U)
        return;

    snprintf(line, sizeof(line), "%-16.16s", message);

    if (strcmp(previous[row], line) != 0)
    {
        LCD_setCursor(0, row);
        LCD_print(line);
        strcpy(previous[row], line);
    }
}

void updateLCD(void)
{
    if (!lcd_ready)
        return;

    char line1[17];
    char line2[17];

    if (mode == HOME)
    {
        snprintf(line1, sizeof(line1), "%s",
                 ventilation_on ? "HOME VENT" : "HOME");

        if (dht_valid)
        {
            snprintf(line2, sizeof(line2), "T:%dC RH:%d%%",
                     temperature, humidity);
        }
        else
        {
            snprintf(line2, sizeof(line2), "T:--C RH:--%%");
        }
    }
    else if (mode == AWAY)
    {
        snprintf(line1, sizeof(line1), "AWAY");
        if (away_countdown)
        {
            uint32_t elapsed = ticks - away_start;
            uint32_t remaining =
                (elapsed >= AWAY_EXIT_TICKS) ? 0U :
                (AWAY_EXIT_TICKS - elapsed + 3U) / 4U;
            snprintf(line2, sizeof(line2), "EXIT %lu s",
                     (unsigned long)remaining);
        }
        else
        {
            snprintf(line2, sizeof(line2), "ARMED");
        }
    }
    else if (mode == ALARM)
    {
        // Both alarm words blink 1 s ON / 1 s OFF.
        if ((((ticks - alarm_start) / LCD_BLINK_TICKS) % 2U) == 0U)
        {
            snprintf(line1, sizeof(line1), "ALARM");
            snprintf(line2, sizeof(line2), "INTRUDER");
        }
        else
        {
            line1[0] = '\0';
            line2[0] = '\0';
        }
    }
    else
    {
        return;
    }

    LCD_writeLine(0, line1);
    LCD_writeLine(1, line2);
}

//==================================================
// Main
//==================================================
int main(void)
{
    setup();
    last_button_time = msTicks - 50U;
    send_status("MODE : HOME\r\n");
    send_status("USART6 BT READY (9600) - send t for RX test\r\n");
    send_status("Send x to clear ALARM\r\n");

    while (1)
    {
        // Button: preserve the existing 50-ms debounce behavior.
        if (button_event)
        {
            button_event = 0;
            uint32_t now = msTicks;
            if ((uint32_t)(now - last_button_time) >= 50U)
            {
                last_button_time = now;

                if (mode == HOME)
                {
                    mode = AWAY;
                    mode_changed();
                }
                else if (mode == AWAY)
                {
                    mode = HOME;
                    mode_changed();
                }
            }
        }

        // Remote alarm acknowledgment: flag set by USART6 (Bluetooth),
        // or USART2 (USB fallback), and handled safely outside the IRQ.
        if (alarm_clear_event)
        {
            alarm_clear_event = 0;
            if (mode == ALARM)
            {
                // Restore AWAY, and give another 5-second exit delay.
                mode = AWAY;
                send_status("ALARM OFF -> back to AWAY\r\n");
                mode_changed();
            }
        }

        // Test Bluetooth RX by sending a single character (e.g. 't')
        // from the laptop's Bluetooth COM port in TeraTerm.
        if (bt_rx_event)
        {
            uint8_t received = bt_rx_char;
            bt_rx_event = 0;
            if (received != '\r' && received != '\n')
            {
                char reply[32];
                snprintf(reply, sizeof(reply), "USART6 RX : %c\r\n", (char)received);
                send_status(reply);
            }
        }

        // Handle pending PIR event outside the interrupt handler.
        handle_pir_event();

        if (mode == HOME)
        {
            home_tasks();
        }
        else if (mode == AWAY)
        {
            away_tasks();
        }

        updateLCD();
        Lighting();
    }
}