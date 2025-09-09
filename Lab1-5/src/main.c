#include "stm32f4xx.h"
#include <stdio.h>
#include "serial_stdio.h"
#include "mpu6050.h"

static void delay_ms(uint32_t ms) {
    SysTick->LOAD = (SystemCoreClock/1000u) - 1u;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (ms--) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
    }
    SysTick->CTRL = 0;
}

int main(void) {
    SystemCoreClockUpdate();


    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER &= ~(3u<<(13*2));
    GPIOC->MODER |=  (1u<<(13*2));
    serial_stdio_init(115200);
    i2c1_init_100k(50000000u);

    if (mpu6050_init() < 0) {
        for (;;) { GPIOC->ODR ^= (1u<<13); delay_ms(150); }
    }

    printf("ax,ay,az,gx,gy,gz,temp\n");

    for (;;) {
        mpu6050_raw_t r;
        if (mpu6050_read_all(&r) == 0) {
            // Print raw values for debugging
            printf("%d,%d,%d,%d,%d,%d,%d\n",
                r.ax, r.ay, r.az, r.gx, r.gy, r.gz, r.temp_raw);
        }


        GPIOC->ODR ^= (1u<<13);
        delay_ms(20);
    }
}