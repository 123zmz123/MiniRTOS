#include "task.h"

#define USER_CODE 1

static volatile UBaseType_t uxTopReadyPriority = tskIDLE_PRIORITY;// init as the lowest priority
TCB_t * volatile pxCurrentTCB = NULL;
List_t pxReadyTasksLists[configMAX_PRIORITIES];// ready tasks
static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;
static volatile UBaseType_t uxTaskNumber = (UBaseType_t)0U;
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;
static volatile TickType_t xTickCount = (TickType_t) 0U; // system tick count 系统时钟，用来记录RTOS运行多久，注意溢出后又从零开始

/* about delay list*/
static List_t xDelayedTaskList1;    /*Delayed List1*/  
static List_t xDelayedTaskList2;    /*Delay List 2*/
static List_t* volatile pxDelayedTaskList; /*point to DelayList*/
static List_t* volatile pxOverflowDelayedTaskList; /*point to DelayList*/
static volatile TickType_t xNextTaskUnlockTime		= ( TickType_t ) 0U; /*get the nearest unlock time*/
static volatile BaseType_t xNumOfOverflows 			= ( BaseType_t ) 0; /*record the overflow time*/

/*
    try to change the current priority of rtos
    add task's TCB to readyList
*/
#define AddTCBToReadyList( pxTCB )																   \
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												   \
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) );
/*
    get the highest priority,
    get the next entry in readtList[currentPriority]
*/

#define taskSELECT_HIGHEST_PRIORITY_TASK()                                              \
    {                                                                                   \
        UBaseType_t uxTopPriority;                                                      \
        portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority);                    \
        listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority])); \
    } /* taskSELECT_HIGHEST_PRIORITY_TASK() */

/*
    if there is no task that need run in current priority, then change to another priority 
*/
	#define taskRESET_READY_PRIORITY( uxPriority )														\
	{																									\
	 if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )\
	 {																								\
			ResetReadyPriority( ( uxPriority ), ( uxTopReadyPriority ) );							\
	 }																								\
	}

#define taskRECORD_READY_PRIORITY(uxPriority) RecordReadyPriority(uxPriority, uxTopReadyPriority)

#define tskRESET_READY_PRIORITY(uxPriority)                           \
    {                                                                 \
        ResetReadyPriority((uxPriority), (uxTopReadyPriority)); \
    }
/*when tickCount overflow we need switch 2 Delayed List*/
#define SwitchDelayList()\
{\
	List_t *pxTemp;\
	pxTemp = pxDelayedTaskList;\
	pxDelayedTaskList = pxOverflowDelayedTaskList;\
	pxOverflowDelayedTaskList = pxTemp;\
	xNumOfOverflows++;\
	prvResetNextTaskUnblockTime();\
}

#if USER_CODE
portCHAR flag1;
portCHAR flag2;
TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;
TaskHandle_t IdleTask_Handle;
TCB_t Task1TCB;
TCB_t Task2TCB;
TCB_t IdleTaskTCB;

#define TASK1_STACK_SIZE 1000
#define TASK2_STACK_SIZE 1000
#define IDLE_TASK_STACK_SIZE 1000

StackType_t Task1Stack[TASK1_STACK_SIZE];
StackType_t Task2Stack[TASK2_STACK_SIZE];
StackType_t IdleTaskStack[IDLE_TASK_STACK_SIZE];

#endif

#if USER_CODE

 void delay (uint32_t count)
{
	for(; count!=0; count--);
}
void Task1_Entry( void *p_arg )
{
	for( ;; )
	{
		flag1 = 0x55;
        tskDelay(2);
        flag1 = 0x77;
        tskDelay(2);
	}
}

void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		flag2 = 1;
        tskDelay(20);
		flag2 = 0;
        tskDelay(20);
	}
}

void runMiniRTOS()
{
    //prvInitialiseTaskLists();
    
    Task1_Handle = tskCreateStaticTask((TaskFunction_t)Task1_Entry,
                                      (char*)"Task1",
                                      (uint32_t)TASK1_STACK_SIZE,
                                      (void*)NULL,
                                      (UBaseType_t)1,
                                      (StackType_t*)Task1Stack,
                                      (TCB_t*)&Task1TCB);

   // vListInsertEnd(&(pxReadyTasksLists[1]), &(((TCB_t *)(&Task1TCB))->xStateListItem));

    Task2_Handle = tskCreateStaticTask((TaskFunction_t)Task2_Entry,
                                      (char*)"Task2",
                                      (uint32_t)TASK2_STACK_SIZE,
                                      (void*)NULL,
                                      (UBaseType_t)2,
                                      (StackType_t*)Task2Stack,
                                      (TCB_t*)&Task2TCB);
    //vListInsertEnd(&(pxReadyTasksLists[2]), &(((TCB_t *)(&Task2TCB))->xStateListItem));

    tskStartScheduler();
}
#endif

static portTASK_FUNCTION(prvIdleTask,pvParameters)
{
    /*prevent compilor warning*/
    (void) pvParameters;
    while(1)
    {
    }
}


void vSetIdleTask()
{
    IdleTask_Handle = tskCreateStaticTask((TaskFunction_t)prvIdleTask,
                                        (char *)"IDLE",
                                        (uint32_t)IDLE_TASK_STACK_SIZE,
                                        (void*)NULL,
                                        (UBaseType_t)tskIDLE_PRIORITY,
                                        (StackType_t*)IdleTaskStack,
                                        (TCB_t*)&IdleTaskTCB);
    //vListInsertEnd(&(pxReadyTasksLists[0]), &(((TCB_t *)(&IdleTaskTCB))->xStateListItem));
}

/*
init  an idle task then start scheduler
*/

void tskStartScheduler(void)
{
    vSetIdleTask();// set an idle task which has the lowest priority
    xNextTaskUnlockTime = portMAX_DELAY;
    xTickCount = (TickType_t)0;
    if(xPortStartScheduler ()!= pdFALSE)
    {

    }
}

#if 1

/*
select one task from current priority
*/
void vTaskSwitchContext(void)
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
}
#endif
#if 0
void vTaskSwitchContext(void)
{

    if (pxCurrentTCB == &IdleTaskTCB)
    {
        if (Task1TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task1TCB;
        }
        else if (Task2TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task2TCB;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (pxCurrentTCB == &Task1TCB)
        {
            if (Task2TCB.xTicksToDelay == 0)
                pxCurrentTCB = &Task2TCB;
            else if (pxCurrentTCB->xTicksToDelay != 0)
                pxCurrentTCB = &IdleTaskTCB;
            else
                return;
        }
        else if (pxCurrentTCB == &Task2TCB)
        {
            if (Task1TCB.xTicksToDelay == 0)
                pxCurrentTCB = &Task1TCB;
            else if (pxCurrentTCB->xTicksToDelay != 0)
                pxCurrentTCB = &IdleTaskTCB;
            else
                return;
        }
    }
}
#endif

//static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /*task entry*/
//									const char * const pcName,              /*task's name*/
//									const uint32_t ulStackDepth,            /*task stack deepth*/
//									void * const pvParameters,              /*task para*/
//									TaskHandle_t * const pxCreatedTask,     /**/
//									TCB_t *pxNewTCB );
                                 

#if(configSUPPORT_STATIC_ALLOCATION)
TaskHandle_t tskCreateStaticTask( TaskFunction_t pxTaskCode, /*Task Entry*/
                                const char* const pcName, /*Task Name*/
                                const uint32_t ulStackDepth,
                                void* const pvParameters,
                                UBaseType_t uxPriority,
                                StackType_t * const puxStackBuffer,/*Stack address*/
                                TCB_t* const pxTaskControlBlock) /**/
{
    TCB_t *pxNewTCB;
    TaskHandle_t xReturn;
    if ((pxTaskControlBlock != NULL) && (puxStackBuffer != NULL))
    {
        pxNewTCB = (TCB_t*) pxTaskControlBlock;
        pxNewTCB->pxStack = (StackType_t*)puxStackBuffer;

        /*create new task*/
        tskInitNewTask(pxTaskCode,
                             pcName,
                             ulStackDepth,
                             pvParameters,
                             uxPriority,
                             &xReturn,
                             pxNewTCB);
        tskAddNewTaskToReadyList(pxNewTCB);
        }
    else
    {
        xReturn=NULL;
    }
return xReturn;
}

#endif 

static void tskInitNewTask( 	TaskFunction_t pxTaskCode,              /*task entry*/
									const char * const pcName,              /*task's name*/
									const uint32_t ulStackDepth,            /*task stack deepth*/
									void * const pvParameters,              /*task para*/
                                    UBaseType_t uxPriority,
									TaskHandle_t * const pxCreatedTask,     /**/
									TCB_t *pxNewTCB )

{
    StackType_t *pxTopOfStack;
    UBaseType_t x;
    /*get the top adrress of stack*/
    pxTopOfStack = pxNewTCB->pxStack + ((ulStackDepth - (uint32_t)1));
    pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );	

    /*put it's name in tcb*/
    for(x=(UBaseType_t)0;x<(UBaseType_t)configMAX_TASK_NAME_LEN;x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x];
        if (pcName[x] == 0x00)
        {
            break;
        }
    }
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';
    
    vListInitialiseItem(&(pxNewTCB->xStateListItem));

    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);

    /*Init Priority*/
    if(uxPriority>=(UBaseType_t)configMAX_PRIORITIES)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    /*init task stack*/
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);

    /*task handler point to tcb*/
    if ((void *)pxCreatedTask != NULL)
    {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB;
    }
}

static void tskAddNewTaskToReadyList(TCB_t *pxNewTCB)
{
    /*enter critical disable interrupts*/
    EnterCritical();/*disable task switch*/
    {
        /*update task number*/
        uxCurrentNumberOfTasks++;
        if(pxCurrentTCB == NULL)
        {
            pxCurrentTCB = pxNewTCB;
            if(uxCurrentNumberOfTasks == (UBaseType_t) 1)
            {
                InitTaskLists();
            }
        }
        else
        {
            if (pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
            {
                pxCurrentTCB=pxNewTCB;
            }
        }
        uxTaskNumber++;
        AddTCBToReadyList(pxNewTCB);
    }
    
    ExitCritical();

}

void InitTaskLists(void)
{
    UBaseType_t uxPriority;
    for (uxPriority = (UBaseType_t)0; uxPriority < (UBaseType_t)configMAX_PRIORITIES; uxPriority++)
    {
        vListInitialise(&(pxReadyTasksLists[uxPriority]));
    }
    vListInitialise(&xDelayedTaskList1);
    vListInitialise(&xDelayedTaskList2);
    pxDelayedTaskList = &xDelayedTaskList1;
    pxOverflowDelayedTaskList = &xDelayedTaskList2;
}
static void prvTaskExitErrot()
{
    for(;;);
}

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters)
{
    pxTopOfStack--;
    *pxTopOfStack =portINITIAL_XPSR; /*xPSR*/
    pxTopOfStack--;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;/**/
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)prvTaskExitErrot;
    pxTopOfStack-=5;/*r12 r3 r2 r1*/
    *pxTopOfStack = (StackType_t)pvParameters;
    pxTopOfStack-=8;
    return pxTopOfStack;
}


BaseType_t xPortStartScheduler(void)
{
/* Make PendSV and SysTick the lowest priority interrupts. */
portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
uxCriticalNesting = 0;// at beginning  CristalNesting initial as zero ;
//xPortSetupTimerInterrupt();
prvStartFirstTask();
}


__asm void prvStartFirstTask(void)
{
    PRESERVE8
      ldr r0, =0xE000ED08
	ldr r0, [r0]
	ldr r0, [r0]

	/* ÉèÖÃÖ÷¶ÑÕ»Ö¸ÕëmspµÄÖµ */
	msr msp, r0
    
	/* Ê¹ÄÜÈ«¾ÖÖÐ¶Ï */
	cpsie i
	cpsie f
	dsb
	isb
	
    /*call svc */
	svc 0  
	nop
	nop
}
/*
    start kernel's 1st task
*/
__asm void vPortSVCHandler(void)
{
    extern pxCurrentTCB;
    PRESERVE8
    ldr r3, =pxCurrentTCB
    ldr r1,[r3]
    ldr r0,[r1];r0->pxCurrentTCB
    ldmia r0!,{r4-r11};pxCurrentTCB->memory contains {r4-r11}
    msr psp,r0; psp-> pxCurrentTCB->pxTopOfStack
    isb ; up code must execute first
    mov r0,#0
    msr basepri,r0;set the basepri of interrupt 
    orr r14,#0xd
    bx lr
}
/*
  task  context switch implement at here .
*/

__asm void xPortPendSVHandler(void)
{
    extern pxCurrentTCB;
    extern vTaskSwitchContext;
    PRESERVE8

    mrs r0, psp /*r0 存储 线程栈的地址*/
    isb 

    ldr r3, =pxCurrentTCB /*r3=&pxCurrentTCB*/
    ldr r2,[r3] /*r2=pxCurrentTCB*/
    stmdb r0!,{r4-r11} /*切换前寄存器入栈*/
    str r0,[r2] /*pxCurrentTCB->pxTopOfStack = psp （thread stack） 结构体的第一个元素位置等于结构体本身的位置*/

    stmdb sp!, {r3,lr} /* r3=&pxCurrentTCB  lr= 返回地址 将这两个存储在 msp 栈之上 是为了 调用 vTaskSwitchContext 做准备*/

    mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
    msr basepri,r0 /*中断编号大于 191的 都不能再干扰pend_sv 中断了， 类似于禁止中断，但禁止的根本不彻底*/
    dsb
    isb
    bl vTaskSwitchContext/*调用 vTaskSwitch*/
    mov r0,#0
    msr basepri, r0/*恢复全局中断*/
    ldmia sp!, {r3,lr } /*恢复调用前的 r3 lr*/

    ldr r1,[r3]
    ldr r0,[r1] /*r0 = pxCurrentTCB->pxTopOfStack*/
    ldmia r0!,{r4-r11} /* r0 所指向栈顶出栈 */
    msr psp, r0 /*psp = pxCurrentTCB->pxTopOfStack*/
    isb 
    bx lr
    nop
}
void xPortSysTickHandler(void)
{
    vPortRaiseBASEPRI();/*disable interrupts*/
    knlIncTick();// 任务调度器
    vPortClearBASEPRIFromISR();/*enable interrupts*/
}

/* trigger an context switch*/
void taskYIELD(void) // 调用pend_sv 中断，进行 context-switch 切换
{

    INTCTRL=0x10000000;// triggle PendSV_Handler
    __dsb(15);
    __isb(15);
}


/*临界区管理*/



/*enable interrupt */
static portFORCE_INLINE void vPortSetBASEPRI (uint32_t ulBASEPRI)
{
    __asm
    {
        msr basepri, ulBASEPRI
    }
}

/*disable interrupt*/
static portFORCE_INLINE void vPortRaiseBASEPRI(void)
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        /*the interrupt which priority is  above configMAX_SYSCALL_INTERRUPT_PRIORITY will disabled*/
        /* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		msr basepri, ulNewBASEPRI
		dsb
		isb
    }
}
/*enable Interrupts*/
static portFORCE_INLINE void vPortClearBASEPRIFromISR(void)
{
    __asm
    {
        /* Set BASEPRI to 0 so no interrupts are masked.  This function is only
		used to lower the mask in an interrupt, so memory barriers are not 
		used. */
		msr basepri, #0
    }
}

/*disable interrupts*/
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI(void)
{
    uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        mrs ulReturn,basepri
        msr basepri,ulNewBASEPRI
        dsb 
        isb
    }
    return ulReturn;
}
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS() vPortSetBASEPRI(0) 
#define portSET_INTERRUPTS_MASK_FROM_ISR() ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)

void EnterCritical(void)// 进入中断
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++; /*全局变量 用来判断是否有其他位置也进行了相关中断*/
    if(uxCriticalNesting == 1)
    {

    }
}

void ExitCritical(void)// 出中断
{
    uxCriticalNesting--;
    if(uxCriticalNesting==0)
    {
        portENABLE_INTERRUPTS();
    }
}

/**/
/*
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    pxTCB = pxCurrentTCB;

    pxTCB->xTicksToDelay = xTicksToDelay;
    taskRESET_READY_PRIORITY( pxTCB->uxPriority );
    taskYIELD();
}*/
#if 0
void vTaskDelay( TickType_t xTicks)
{

    (*pxCurrentTCB).xTicksToDelay = xTicks;
    taskRESET_READY_PRIORITY( (*pxCurrentTCB).uxPriority );
    taskYIELD();
}
#endif
void tskDelay( TickType_t xTicks)// 任务延时，同时切换到其他任务
{
 //   TCB_t *pxTCB = NULL;
   // pxTCB = pxCurrentTCB;
    AddCurrentTaskToDelayList(xTicks);//将当前任务添加到延时任务链表，同时计算出延时时间
    taskYIELD();//触发中断
}
#if 0
void knlIncTick(void)
{
    TCB_t *pxTCB = NULL;
    BaseType_t i = 0;
    /*update systme tick  time base*/
    const TickType_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;
    if(xTickCount == 16402)
    {
       xTickCount=0 ;
    }

    for(i=0;i<configMAX_PRIORITIES;i++)
    {
        pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
        if(pxTCB->xTicksToDelay > 0 )
        {
            (pxTCB->xTicksToDelay)--;
            if (pxTCB->xTicksToDelay == 0)
            {
                taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
            }
        }
      
    }
    taskYIELD();
}
#endif
// 将当前TCB加入到DelayList之中
static void AddCurrentTaskToDelayList(TickType_t xTicksToWait)
{
    TickType_t xTimeToWake;
    const TickType_t xConstTickCount = xTickCount;// xTickCount 随systic 中断而不断增加，系统计时器
    if(uxListRemove(&(pxCurrentTCB->xStateListItem)) == (UBaseType_t)0)// 一旦当前优先级下的readyList 不再有其他任务
    {
        ResetReadyPriority(pxCurrentTCB->uxPriority,
                                uxTopReadyPriority); // 系统当前优先级下降到其他优先级
    }
    xTimeToWake = xConstTickCount + xTicksToWait;// 确认该任务的唤醒时基

    listSET_LIST_ITEM_VALUE(&(pxCurrentTCB->xStateListItem),
                            xTimeToWake);// xStateListItem 是任务的唤醒时间，同时也用作排序。
    
    /*time count overflow*/
    if(xTimeToWake < xConstTickCount)
    {
        vListInsert(pxOverflowDelayedTaskList,&(pxCurrentTCB->xStateListItem));
    }
    else
    {
        vListInsert(pxDelayedTaskList, &(pxCurrentTCB->xStateListItem));
        if (xTimeToWake < xNextTaskUnlockTime)//一旦有更为紧迫的唤醒任务后
        {
            xNextTaskUnlockTime = xTimeToWake; // 唤醒时间更新
        }
    }
}

void knlIncTick(void)// 每次systic更新后都要进行调用 此处实际上就是调度器
{
   TCB_t* pxTCB;
   TickType_t xItemValue;
   volatile TickType_t xConstTickCount = xTickCount + 1;
   xTickCount = xConstTickCount;

   if (xConstTickCount == (TickType_t)0U)
   {
       SwitchDelayList(); // xTickCount 重新清零后需要切换DelayList
   }

   if(xConstTickCount >= xNextTaskUnlockTime) // 一旦有任务延时状态取消时
   {
       for(;;)
       {
           if(listLIST_IS_EMPTY(pxDelayedTaskList)!= pdFALSE)
           {
               xNextTaskUnlockTime = portMAX_DELAY;
               break;
           }
           /*delay list is not empty*/
           else 
           {
               pxTCB = (TCB_t*) listGET_OWNER_OF_HEAD_ENTRY(pxDelayedTaskList);
               xItemValue = listGET_LIST_ITEM_VALUE(&(pxTCB->xStateListItem));
               if (xConstTickCount < xItemValue) // xItemValue实际上指代延时时间
               {
                   xNextTaskUnlockTime = xItemValue;
                   break;
               }
               (void)uxListRemove(&(pxTCB->xStateListItem));
               AddTCBToReadyList(pxTCB);
           }
       }
   }
   taskYIELD();
}
#if 0
void knlIncTick(void)
{
    TCB_t *pxTCB = NULL;
    BaseType_t i = 0;
    /*update systme tick  time base*/
    const TickType_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;
    if(xTickCount == 16402)
    {
       xTickCount=0 ;
    }

    for(i=0;i<configMAX_PRIORITIES;i++)
    {

        if(!listLIST_IS_EMPTY(&pxReadyTasksLists[i]))
        {
            pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
            if (pxTCB->xTicksToDelay > 0)
            {
                (pxTCB->xTicksToDelay)--;

                if (pxTCB->xTicksToDelay == 0)
                {
                    taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
                }
            }
        }
    }
    taskYIELD();
}
#endif

void xPortSetupTimerInterrupt(void)
{
     portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | 
                                  portNVIC_SYSTICK_INT_BIT | 
                                  portNVIC_SYSTICK_ENABLE_BIT );
}


static void prvResetNextTaskUnblockTime( void )
{
    TCB_t *pxTCB;

	if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
	{
		/* The new current delayed list is empty.  Set xNextTaskUnblockTime to
		the maximum possible value so it is	extremely unlikely that the
		if( xTickCount >= xNextTaskUnblockTime ) test will pass until
		there is an item in the delayed list. */
		xNextTaskUnlockTime = portMAX_DELAY;
	}
	else
	{
		/* The new current delayed list is not empty, get the value of
		the item at the head of the delayed list.  This is the time at
		which the task at the head of the delayed list should be removed
		from the Blocked state. */
		( pxTCB ) = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
		xNextTaskUnlockTime = listGET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
	}
}
