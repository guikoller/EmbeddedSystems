#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"   /* SystemCoreClock */
#include <stdint.h>

/* ----------------------------------------------------------
 * Kernel
 * ---------------------------------------------------------- */
#define configUSE_PREEMPTION                         1
#define configUSE_IDLE_HOOK                          0
#define configUSE_TICK_HOOK                          0
#define configCPU_CLOCK_HZ                           ( SystemCoreClock )
#define configTICK_RATE_HZ                           ( 1000 )
#define configMAX_PRIORITIES                         ( 5 )
#define configMINIMAL_STACK_SIZE                     ( (unsigned short)128 )
#define configMAX_TASK_NAME_LEN                      ( 16 )
#define configUSE_16_BIT_TICKS                       0
#define configIDLE_SHOULD_YIELD                      1
#define configUSE_TIME_SLICING                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION      1

/* ----------------------------------------------------------
 * Memória
 * ---------------------------------------------------------- */
#define configTOTAL_HEAP_SIZE                        ( (size_t)(32 * 1024) )
#define configSUPPORT_DYNAMIC_ALLOCATION             1
#define configSUPPORT_STATIC_ALLOCATION              0
#define configUSE_NEWLIB_REENTRANT                   0

/* ----------------------------------------------------------
 * Recursos
 * ---------------------------------------------------------- */
#define configUSE_MUTEXES                            1
#define configUSE_TASK_NOTIFICATIONS                 1
#define configUSE_TIMERS                             1
#define configTIMER_TASK_PRIORITY                    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                     10
#define configTIMER_TASK_STACK_DEPTH                 (configMINIMAL_STACK_SIZE * 2)
#define configQUEUE_REGISTRY_SIZE                    8

/* ----------------------------------------------------------
 * Debug / Hooks
 * ---------------------------------------------------------- */
#define configCHECK_FOR_STACK_OVERFLOW               2
#define configUSE_MALLOC_FAILED_HOOK                 1

/* Ative as APIs usadas */
#define INCLUDE_vTaskDelay                           1
#define INCLUDE_vTaskDelayUntil                      1
#define INCLUDE_vTaskDelete                          1
#define INCLUDE_vTaskSuspend                         1
#define INCLUDE_xTaskGetSchedulerState               1
#define INCLUDE_xTaskGetCurrentTaskHandle            1
#define INCLUDE_xTimerPendFunctionCall               1

/* ----------------------------------------------------------
 * Prioridades NVIC (STM32F4: 4 bits)
 * ---------------------------------------------------------- */
#define configPRIO_BITS                              4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY              ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY      << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY         ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* ----------------------------------------------------------
 * Mapear handlers exigidos pelo port
 * ---------------------------------------------------------- */
#define vPortSVCHandler                              SVC_Handler
#define xPortPendSVHandler                           PendSV_Handler
#define xPortSysTickHandler                          SysTick_Handler

/* ----------------------------------------------------------
 * FPU
 * ---------------------------------------------------------- */
#define configENABLE_FPU                             1
#define configUSE_TASK_FPU_SUPPORT                   1

/* ----------------------------------------------------------
 * Assert
 * ---------------------------------------------------------- */
void vAssertCalled(const char *file, int line);
#define configASSERT(x) if ((x) == 0) { vAssertCalled(__FILE__, __LINE__); }

#endif /* FREERTOS_CONFIG_H */
