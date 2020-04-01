
#ifndef __LED_H
#define __LED_H
#include "stm32f4xx_hal.h"

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

void GreenLedToggle();
void GreenLedBlink(void);
void GreenLedOn(void);
void GreenLedOff(void);
void GreenLedOn_Thread(void);
void GreenLedOff_Thread(void);
void test_Context(void);
void threadStackInit();
void Task1();
void Task2();
void Task3();
extern int32_t semaphore1, semaphore2;

#endif // !1LED_CONTROL
