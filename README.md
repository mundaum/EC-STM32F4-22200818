# Embedded Controller — STM32F411RE

**Embedded Controller | Handong Global University**

**Author:** Daum Moon  
**Student ID:** 22200818  
**MCU:** STM32F411RE (Nucleo-64)  
**Development Environment:** VS Code, PlatformIO, CMSIS

---

## Introduction

This repository contains my Embedded Controller laboratory assignments and tutorial projects at Handong Global University.

In this course, I learned how to control hardware using the STM32F411RE microcontroller.

I implemented GPIO control, external interrupts, USART communication, stepper motor control, timers, and PWM.

## Repository Structure

| Directory | Description |
|---|---|
| [include](./include) | STM32 library files |
| [lab](./lab) | Laboratory assignments |
| [tutorial](./tutorial) | Tutorial and PreLAB projects |
| [lib](./lib) | Additional libraries |

---

## Laboratory Projects

### LAB 01. GPIO Digital I/O — LED & Photosensor

**Source Code:** [LAB_GPIO_DIO_LED_Photosensor.c](./lab/LAB_GPIO_DIO_LED_Photosensor.c)

This experiment introduces GPIO digital input and output control.

I used GPIO pins to read input signals from a photosensor and control an LED.

**Main Topics**
- GPIO input and output modes
- Digital input reading
- LED output control
- Photosensor interface

### LAB 02. GPIO Digital I/O — Multiple LEDs

**Source Code:** [LAB_GPIO_DIO_multiLED.c](./lab/LAB_GPIO_DIO_multiLED.c)

In this experiment, I controlled multiple LEDs using the STM32 GPIO pins.

I practiced configuring digital output pins and controlling LED states through C programming.

**Main Topics**
- Multiple GPIO outputs
- Digital HIGH and LOW signals
- LED control

### LAB 03. GPIO — 7-Segment Display

**Source Code:** [LAB_GPIO_7segment.c](./lab/LAB_GPIO_7segment.c)

In this experiment, I controlled a 7-segment display using GPIO output signals.

I learned how to display numbers by controlling the individual segments.

**Main Topics**
- 7-segment display control
- GPIO output configuration
- Segment patterns for numbers

### LAB 04. External Interrupt — EXTI

**Source Code:** [LAB_EXTI.c](./lab/LAB_EXTI.c)

In this experiment, I used external interrupts to respond to button inputs.

I also used SysTick timing to control the operation of a 7-segment display.

**Main Topics**
- External interrupt configuration
- EXTI registers and interrupt handlers
- SysTick timer
- Button-controlled counter operation

### LAB 05. USART — LED Control

**Source Code:** [LAB_USART_LED.c](./lab/LAB_USART_LED.c)

In this experiment, I implemented USART serial communication using the STM32F411RE.

I practiced sending and receiving serial data and controlling an LED through communication commands.

**Main Topics**
- USART initialization
- Baud rate configuration
- Serial data transmission and reception
- LED control

### LAB 06. USART — Bluetooth Communication

**Source Code:** [LAB_USART_Bluetooth.c](./lab/LAB_USART_Bluetooth.c)

In this experiment, I used an HC-06 Bluetooth module for wireless serial communication.

The STM32 received commands through USART and used them to control connected hardware.

**Main Topics**
- HC-06 Bluetooth module
- USART1 communication
- Wireless command reception
- Hardware control using Bluetooth

### LAB 07. Stepper Motor Control

**Source Code:** [LAB_Stepper_Motor.c](./lab/LAB_Stepper_Motor.c)

In this experiment, I controlled a 28BYJ-48 stepper motor using an ULN2003 motor driver.

I implemented Full-stepping and Half-stepping using finite state machines (FSMs).

The motor was tested in both clockwise (CW) and counterclockwise (CCW) directions.

**Main Features**
- Full-stepping: 2048 steps per revolution
- Half-stepping: 4096 steps per revolution
- CW and CCW rotation
- RPM-based speed control
- Emergency stop using the B1 USER button
- Maximum working speed setting: 14 RPM

**Demo Video:** [Stepper Motor Experiment](https://youtu.be/-eZQ6fAV6YA)

---

## Tutorial & PreLAB

### Timer Interrupt

Implemented timer initialization and period configuration using STM32 timer registers.

**Main Functions**
- `TIM_init()`
- `TIM_init_us()`
- `TIM_period_us()`

### PWM Control

Implemented PWM configuration and duty ratio control using STM32 hardware timers.

**Main Functions**
- `PWM_init_config()`
- `PWM_duty()`

---

## Development Tools

- STM32 Nucleo-F411RE
- C Programming Language
- Visual Studio Code
- PlatformIO
- CMSIS
- ST-LINK

## References

- [Embedded Controller Course Materials](https://ykkim.gitbook.io/ec/)
- [Embedded Controller Student Repository](https://github.com/ykkimhgu/EC-student)

---

*This repository documents my Embedded Controller coursework and experiments based on the provided laboratory materials.*
