		
    AREA | .text|,CODE,READONLY,ALIGN=2
		THUMB 
    PRESERVE8
		EXTERN currentPt
    EXTERN uwTick
    EXTERN uwTickFreq
    IMPORT HAL_IncTick
    IMPORT GreenLedToggle
    IMPORT osSchedulerRoundRobin 
    ;EXPORT SysTick_Handler ;使当前函数可以被其他外部文件看到
    ;EXPORT PendSV_Handler
   ; EXPORT osSchedulerLauch

PendSV_Handler
  B CONTEXT_SWITCH_C






;任务切换写在 SysTick_Handler 中断中，此中断属于ARM内核本身，不属于各大芯片厂商，配置简单反而有利于，系统移植。
SysTick_Handler        ; SysTick 中断只属于 cortex 系列
  push {lr}            ;将当前函数返回地址(lr寄存器中存储)放入栈中，然后才可以进入新函数
  BL HAL_IncTick       ; HAL 中的计数器函数
  pop {lr}             ;从计数器函数返回后，需要把函数返回地址放到，lr寄存器中,使得中断函数可以正确返回。
  B CONTEXT_SWITCH_C   ;跳转到上下文切换函数
  
CONTEXT_SWITCH ; 任务切换
  CPSID I           ;禁止中断
  PUSH{R4 - R11}    ;将当前任务中的 R4-R11 寄存器内容存储到栈中，R0-R3根据约定只存储函数的参数，不参与中间过程的处理，因此没有必要做特殊处理。
  LDR R0,=currentPt ; currentPtr的地址被放置到R0寄存器中。
  LDR R1,[R0]       ;R1 = currentPtr->StackPtr
  STR SP,[R1]       ;SP = currentPtr->StackPtr
  LDR R1,[R1,#4]    ;R1=currentPtr->nextPtr
  STR R1,[R0]       ;currentPtr=currentPtr->nextPtr 
  LDR SP,[R1]       ;SP=currentPtr->StackPtr 
  POP {R4 - R11}    ;R4-R11 = currentPtr->StackPtr contains
  CPSIE I           ;开启中断
  BX LR             ;函数返回 LR所指向的地址中(中断中实际上并不是)但这条命令的确可以结束中断调用。

CONTEXT_SWITCH_C ; 任务切换 需要调用C函数，来简化相关。
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

osSchedulerLauch ; 此函数已经废弃，在osKernel.c中有此函数的相同实现，我也是尽可能把函数都写进一个C文件中
    LDR     R0, =currentPt         
    LDR     R2, [R0]       ; R2 =currentPt       
    LDR     SP, [R2]       ;SP = currentPt->stackPt    
    POP     {R4-R11}          
    POP     {R0-R3}            
    POP     {R12}
    ADD     SP,SP,#4           
    POP     {LR}               
    ADD     SP,SP,#4         
    CPSIE   I                 
    BX      LR                 




ALIGN
END

  
  