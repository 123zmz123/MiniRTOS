#include    "LedControl.h"
#include"osKernel.h"
uint32_t count1,count2,count3;
uint32_t volatile context_flag = 1;
uint32_t stkLedOffStack[40];
uint32_t stkLedOnStack[40];
uint32_t *ptrLedOffStack = &stkLedOffStack[40];
uint32_t *ptrLedOnStack = &stkLedOnStack[40];

int32_t semaphore1, semaphore2;
void threadStackInit()
{
    *(--ptrLedOffStack) = (1U << 24U);                   /*XPSR program status register*/
    *(--ptrLedOffStack) = (uint32_t)&GreenLedOff_Thread; /*PC program counter*/
    *(--ptrLedOffStack) = 0x0000000DU; /*LR link register*/
    *(--ptrLedOffStack) = 0x0000000EU; /*R12*/
    *(--ptrLedOffStack) = 0x0000000AU; /*R3*/
    *(--ptrLedOffStack) = 0x0000000EU; /*R2*/
    *(--ptrLedOffStack) = 0x0000000AU; /*R1*/
    *(--ptrLedOffStack) = 0x0000000DU; /*R0*/

    *(--ptrLedOnStack)=(1U<<24U);/*xPSR*/
    *(--ptrLedOnStack)=(uint32_t)&GreenLedOn_Thread;/*PC*/
    *(--ptrLedOnStack) = 0x0000000BU; /*LR link register*/
    *(--ptrLedOnStack) = 0x0000000EU; /*R12*/
    *(--ptrLedOnStack) = 0x0000000BU; /*R3*/
    *(--ptrLedOnStack) = 0x0000000CU; /*R2*/
    *(--ptrLedOnStack) = 0x0000000AU; /*R1*/
    *(--ptrLedOnStack) = 0x0000000FU; /*R0*/




}
void GreenLedToggle()
{
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

void GreenLedBlink()
{
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(100); /* USER CODE END WHILE */
}

void GreenLedOn()
{
    HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);
  
}

void GreenLedOff()
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

void GreenLedOn_Thread()
{
    while (1)
    {
        GreenLedOn();
        HAL_Delay(100);
    }
    
}
void GreenLedOff_Thread()
{
    while (1)
    {
        GreenLedOff();
        HAL_Delay(100);
    }
    
}

// 此函数被用来初步测试，上下文切换的作用，在main函数中调用此函数，初始化时，修改 context_flag
// 在内存中的数值，
// 例如: context_flag = 0 会看到 绿灯开
// 例如: context_flag = 1 会看到 绿灯关
void test_Context(void)
{
    if(context_flag == 0)
    {
        GreenLedOn_Thread();
    }
    else
    {
        GreenLedOff_Thread();
    }

}

/*
Task中会要加入信号量
*/

void Task1()
{
    while(1)
    {
        osSignalWait(&semaphore1);
        count1++;
        osSignalSet(&semaphore2);
    }
}

void Task2()
{
    while(1)
    {
        osSignalWait(&semaphore2);
        count2++;
        osSignalSet(&semaphore1);
    }
}

void Task3()
{
    while(1)
    {
        count3++;
    }
}

