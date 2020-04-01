#ifndef _TASK_H
#define _TASK_H
#include "portmacro.h"
#include "config.h"
#include"list.h"
#define INTCTRL (*((volatile uint32_t *)0xE000ED04)) // cortex-m4 Interrupt control Register

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT 			( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT		( 1UL << 25UL )
#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )

#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

#define portINITIAL_XPSR			        ( 0x01000000 )
#define portSTART_ADDRESS_MASK				( ( StackType_t ) 0xfffffffeUL )

#define portINLINE __inline
#ifndef portFORCE_INLINE
#define portFORCE_INLINE __forceinline
#endif 



#if  (configUSE_PORT_OTIMISED_TASK_SELECTION == 1)
#define RecordReadyPriority(uxPriority, uxReadyPriorities) (uxReadyPriorities) |= (1UL << (uxPriority))
#define ResetReadyPriority(uxPriority, uxReadyPriorities) (uxReadyPriorities) &= ~(1UL << (uxPriority))
#define portGET_HIGHEST_PRIORITY(uxTopPriority, uxReadyPriorities) uxTopPriority = (31UL - (uint32_t)__clz((uxReadyPriorities)))


#endif 

typedef void * TaskHandle_t;
typedef void (*TaskFunction_t)( void * );
typedef  struct taskTaskControlBlock
{
    volatile StackType_t *pxTopOfStack;// 栈顶
    ListItem_t xStateListItem; /*the tcb is belong in an List*/
    StackType_t *pxStack;//栈边界
    char pcTaskName[configMAX_TASK_NAME_LEN];//任务名称
    TickType_t xTicksToDelay;//任务延时时间，已经弃用
    UBaseType_t uxPriority;// 任务优先级
}tskTCB;
typedef tskTCB TCB_t;



TaskHandle_t tskCreateStaticTask( TaskFunction_t pxTaskCode, /*Task Entry*/
                                const char* const pcName, /*Task Name*/
                                const uint32_t ulStackDepth,
                                void* const pvParameters,
                                UBaseType_t uxPriority,
                                StackType_t * const puxStackBuffer,/*Stack address*/
                                TCB_t* const pxTaskBuffer);

static void tskInitNewTask( 	TaskFunction_t pxTaskCode,              /*task entry*/
									const char * const pcName,              /*task's name*/
									const uint32_t ulStackDepth,            /*task stack deepth*/
									void * const pvParameters,              /*task para*/
                                    UBaseType_t uxPriority,
									TaskHandle_t * const pxCreatedTask,     /**/
									TCB_t *pxNewTCB );

void InitTaskLists(void);
static void prvTaskExitErrot();

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters);
BaseType_t xPortStartScheduler(void);
__asm void prvStartFirstTask(void);
void tskStartScheduler(void);
void vTaskSwitchContext(void);
__asm void vPortSVCHandler(void);
__asm void xPortPendSVHandler(void);
void taskYIELD(void);
void runMiniRTOS(void);
void EnterCritical(void);
void ExitCritical(void);
static portFORCE_INLINE void vPortRaiseBASEPRI(void);
static portFORCE_INLINE void vPortSetBASEPRI (uint32_t ulBASEPRI);
static portFORCE_INLINE void vPortClearBASEPRIFromISR(void);
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI(void);
void knlIncTick(void);
void xPortSysTickHandler(void);
void tskDelay(const TickType_t xTicksToDelay);
static void tskAddNewTaskToReadyList(TCB_t *pxNewTCB);
void xPortSetupTimerInterrupt(void);
static void AddCurrentTaskToDelayList(TickType_t xTicksToWait);
static void prvResetNextTaskUnblockTime( void );
#endif