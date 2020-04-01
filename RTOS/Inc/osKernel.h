#ifndef __OS_KERNEL_H
#define __OS_KERNEL_H
#include"stm32f4xx_hal.h"
#include"core_cm4.h"

#include "portmacro.h"
#include "config.h"
#include"list.h"
#include"task.h"
#include"prjdef.h"


void osKernelLanch(uint32_t quanta);
void osKernelInit(void);
uint8_t osKernelAddThreads(void(*task0)(void),
                           void(*task1)(void),
                           void(*task2)(void));
void osSchedulerRoundRobin(void);
void osSemaphoreInit(int32_t *semaphore, int32_t value);
void osSignalWait(int32_t *semaphore);
void osSignalSet(int32_t *semaphore);
void osThreadYield(void);
void SysTick_Handler(void);
void PendSV_Handler(void);
void SVC_Handler(void);
#endif