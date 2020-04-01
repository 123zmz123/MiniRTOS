#include "osKernel.h"
#include"list.h"
#include "config.h"

// In the default mode ， Nucleao f401 Systic interrupt is 1khz. 1ms
// 对于 Nucleo F401 来说， 默认模式下 systic 已经被设置为 1khz , 周期为1ms因此，我不想在去搞什么os切换时间了，作为一个演示类操作系统没什么意义。
//Scheduler
//TCB
//Stack
#define NUM_OF_THREADS 3// the max task number that our os supported
#define STACKSIZE 100// Each task's Stack Size
#define _1Mhz 1000000 
#define BUS_FREQ    16*_1Mhz
#define PERIOD 100
#define INTCTRL (*((volatile uint32_t *)0xE000ED04)) // cortex-m4 Interrupt control Register
#define portNVIC_SYSPRI2_REG (*((volatile uint32_t *)0xe000ed20))
#define portNVIC_PENDSV_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 24UL)
uint32_t MILLIS_PRESCALER = 0;
uint32_t period_cnt=0;
void osSchedulerLauch();
void ToggleTask();
/*task control block define*/
struct tcb
{
    int32_t *stackPt;//current Task's Stack Pointer
    struct tcb *nextPt;//Point to next Task's "Task control block"
};

typedef struct tcb tcbType; 
;

tcbType tcbs[NUM_OF_THREADS];// alloc memory to store tcb(task control blocks)
tcbType *currentPt; // point to current tcb (task control block)
int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];//task STACKS defined in memory


/*
Init number i task's stack
*/
void osKernelStackInit(int i)
{
    tcbs[i].stackPt = &TCB_STACK[i][STACKSIZE-16];//point to the top of stack, each context we will save 16 registers
    /*in each context cortext m4 will help us to save below registers automatically */
    TCB_STACK[i][STACKSIZE-1]=0x01000000;//init psr register.
    TCB_STACK[i][STACKSIZE-3]=0x14141414;//LR (R14)
    TCB_STACK[i][STACKSIZE-4]=0x12121212;//R12
    TCB_STACK[i][STACKSIZE-5]=0x03030303;//R3
    TCB_STACK[i][STACKSIZE-6]=0x02020202;//R2
    TCB_STACK[i][STACKSIZE-7]=0x01010101;//R1
    TCB_STACK[i][STACKSIZE-8]=0x00000000;//R0
    /*we must save manually*/
    TCB_STACK[i][STACKSIZE-9]=0x11111111;//R11
    TCB_STACK[i][STACKSIZE-10]=0x10101010;//R10
    TCB_STACK[i][STACKSIZE-11]=0x09090909;//R9
    TCB_STACK[i][STACKSIZE-12]=0x08080808;//R8
    TCB_STACK[i][STACKSIZE-13]=0x07070707;//R7
    TCB_STACK[i][STACKSIZE-14]=0x06060606;//R6
    TCB_STACK[i][STACKSIZE-15]=0x05050505;//R5
    TCB_STACK[i][STACKSIZE-16]=0x04040404;//R4
}

/*
    add 3 tasks to our kernel
*/
uint8_t osKernelAddThreads(
void(*task0)(void),
void(*task1)(void),
void(*task2)(void))
{
    __disable_irq();// disable interrupt
    //init tcbs 
    tcbs[0].nextPt = &tcbs[1];
    tcbs[1].nextPt = &tcbs[2];
    tcbs[2].nextPt = &tcbs[0];

    osKernelStackInit(0);//init 1st task
    TCB_STACK[0][STACKSIZE-2]=(int32_t)(task0);//stack->lr(register) = task0 address

    osKernelStackInit(1);//init 2nd task
    TCB_STACK[1][STACKSIZE-2]=(int32_t)(task1);//stack->lr(register) = task1 address

    osKernelStackInit(2);//init 3rd task
    TCB_STACK[2][STACKSIZE-2]=(int32_t)(task2);// stack->lr(register) = task2 address
    currentPt = &tcbs[0];//currentPt=task0's stack pointer

    __enable_irq();//enable interrupt

    return 1;
}

void osKernelInit(void)
{
    __disable_irq();
    MILLIS_PRESCALER = (BUS_FREQ/1000);
}
/* 堆栈中（某一块特定内存）与寄存器对应关系，调用 osSchedulerLauch后，堆栈内容按此规律被放到arm寄存器中，堆栈指针一步步指向新的栈顶
----------------------
|         R4         |------------------>StackPointer(堆栈指针)
----------------------  
|         R5         |
----------------------  
|         R6         |
----------------------  
|         R7         |
----------------------  
|         R8         |
----------------------  
|         R9         |
----------------------  
|         R10        |
----------------------  
|         R11        |
----------------------  
|         R0         |
----------------------  
|         R1         |
----------------------  
|         R2         |
----------------------  
|         R3         |
----------------------  
|         R12        |
----------------------  
|         SP         |
----------------------  
|         LR (r14)   |
----------------------  
|         PC         |
----------------------
|         New(R4)    |------------------->New StackPointer(堆栈指针)
----------------------

*/
__asm void osSchedulerLauch()        // lauch the first task                
{
    extern currentPt

  CPSID I;disable interrupt
        ldr r0,=currentPt;r0=currentPt address
        ldr r2,[r0];r2=currentPt
        ldr sp, [r2] ;SP=currentPtr
        pop {r4-r11}
        pop {r0-r3}
        pop {r12}
        add sp,sp,#4
        pop {lr}
        add sp,sp,#4
        CPSIE I
        bx lr
}
void osKernelLanch(uint32_t quanta)// start the kernel
{
    //HAL_SYSTICK_Config(MILLIS_PRESCALER*quanta);
    osSchedulerLauch();
}

void osSchedulerRoundRobin(void)// round robin scheduler , call by systic interrupt
{
    period_cnt++;
    if (period_cnt == PERIOD)
    {
        (*ToggleTask)();
        period_cnt=0;
    }
    currentPt = currentPt->nextPt;
}

void osThreadYield(void)
{
    /*
    trigger the systic interrupt
    */
    SysTick->VAL = 0;
    INTCTRL = 0x04000000; // Trigger Systick
}

void osSemaphoreInit(int32_t *semaphore, int32_t value)
{
    (*semaphore)=value;
}
void osSignalSet(int32_t *semaphore)
{
    __disable_irq();
    (*semaphore)+=1;
    __enable_irq();
}

void osSignalWait(int32_t *semaphore)
{
    __disable_irq();
    while (*semaphore<=0)
    {
        __disable_irq();
        osThreadYield();// trigger the systic interrupt then context switch
        __enable_irq();
    }
    (*semaphore)-=1;
    __enable_irq();
    

}

void ToggleTask()// toggle the led to indicate the context switch happened.
{
    GreenLedToggle();
}

void SysTick_Handler(void)
{
   // HAL_IncTick();h
#if USE_OS
    INTCTRL=0x10000000;// triggle PendSV_Handler
#elif USE_MINI_RTOS
xPortSysTickHandler();
#endif 
}

__asm void osContextSwitch(void)
{
  extern osSchedulerRoundRobin
  extern currentPt
  PRESERVE8
  CPSID I
  PUSH{R4 - R11}
  LDR R0,=currentPt
  LDR R1,[R0]       ;	R1 = currentPtr->StackPtr
  STR SP,[R1]       ;	currentPtr->StackPtr = SP
  PUSH {R0,LR}
  BL osSchedulerRoundRobin
  POP {R0,LR}
  LDR R1,[R0]
  LDR SP,[R1]       ;SP=currentPtr->StackPtr 
  POP {R4 - R11}      ;currentPtr->StackPtr storerage to R4-R11
  CPSIE I 
  BX LR
}

void PendSV_Handler(void)
{
#if USE_OS
    osContextSwitch();
#elif USE_MINI_RTOS
    xPortPendSVHandler();
#endif
}


void SVC_Handler(void)
{
    #if USE_OS

    #elif USE_MINI_RTOS

    vPortSVCHandler();

    #endif
}
