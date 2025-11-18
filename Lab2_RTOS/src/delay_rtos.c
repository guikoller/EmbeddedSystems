#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay_rtos.h"

static inline int rtos_running(void) {
    return xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
}

void delay_init(void) {
    /* Enable DWT CYCCNT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if defined(DWT_LAR) /* some MCUs lock DWT */
    DWT->LAR = 0xC5ACCE55;
#endif
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (SystemCoreClock / 1000000u) * us;
    while ((DWT->CYCCNT - start) < ticks) { __NOP(); }
}

void delay_ms(uint32_t ms) {
    if (rtos_running()) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        while (ms--) delay_us(1000u);
    }
}
