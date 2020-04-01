#ifndef _CONFIG_H
#define _CONFIG_H

/*task controls define */
#define tskIDLE_PRIORITY (0)
/*---------------------*/
#define configUSE_16_BIT_TICKS		              0
#define configMAX_TASK_NAME_LEN		            ( 16 )
#define configSUPPORT_STATIC_ALLOCATION           1
#define configMAX_PRIORITIES		            ( 6 )

#define configKERNEL_INTERRUPT_PRIORITY 		255   
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191   
#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )

#define configCPU_CLOCK_HZ			( ( unsigned long ) 25000000 )	
#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
#define configTICK_RATE_HZ			( ( TickType_t ) 1000)


#ifndef configUSE_PORT_OTIMISED_TASK_SELECTION
#define  configUSE_PORT_OTIMISED_TASK_SELECTION 1
#endif
#endif