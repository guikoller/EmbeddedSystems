#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "serial_stdio.h"
#include "mpu6050.h"
#include "st7789.h"
// NÃO usar delay_rtos aqui antes do scheduler
// #include "delay_rtos.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ==== FreeRTOS hooks ==== */

void vAssertCalled(const char *file, int line) {
    (void)file;
    (void)line;
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

/* ==== busy delay só para antes do scheduler ==== */

static void busy_delay_cycles(volatile uint32_t cycles) {
    while (cycles--) {
        __asm volatile ("nop");
    }
}

/* aproximado: assume SystemCoreClock correto */
static void busy_delay_ms(uint32_t ms) {
    uint32_t cycles_per_ms = SystemCoreClock / 8000u; // aproximadamente
    busy_delay_cycles(cycles_per_ms * ms);
}

/* ==== defines ==== */

#define LED_RED_PIN     2
#define LED_GREEN_PIN   3
#define LED_BLUE_PIN    4
#define BUTTON_PIN      0

#define ACCEL_BRAKE_THRESHOLD    -8000
#define ACCEL_CRASH_THRESHOLD    20000
#define GYRO_CURVE_THRESHOLD     8000

#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_WHITE     0xFFFF
#define COLOR_BLACK     0x0000

/* ==== shared state ==== */

static volatile uint32_t total_events = 0;
static uint8_t button_prev = 0;

static TickType_t led_red_timer   = 0;
static TickType_t led_green_timer = 0;
static TickType_t led_blue_timer  = 0;

/* queue para eventos (MPU -> EventTask) */
static QueueHandle_t mpuQueue = NULL;

/* ==== HC-12 / USART2 ==== */

static void hc12_init(uint32_t baudrate) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER &= ~((3u<<(2*2)) | (3u<<(3*2)));
    GPIOA->MODER |=  ((2u<<(2*2)) | (2u<<(3*2)));
    GPIOA->AFR[0] &= ~((0xFu<<(2*4)) | (0xFu<<(3*4)));
    GPIOA->AFR[0] |=  ((7u<<(2*4)) | (7u<<(3*4)));

    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;

    /* PCLK1 real: Blackpill F411 -> ~ SystemCoreClock / 2 (100 MHz / 2 = 50 MHz) */
    uint32_t apb1 = SystemCoreClock / 2u;
    USART2->BRR = apb1 / baudrate;        /* 50_000_000 / 9600 ≈ 5208 (0x1458) */

    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

static void hc12_send_char(char c) {
    while ((USART2->SR & USART_SR_TXE) == 0u) {
    }
    USART2->DR = (uint16_t)c;
}

static void hc12_send_string(const char *s) {
    while (*s) {
        hc12_send_char(*s++);
    }
}

/* ==== GPIO ==== */

static void leds_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~((3u<<(LED_RED_PIN*2)) |
                      (3u<<(LED_GREEN_PIN*2)) |
                      (3u<<(LED_BLUE_PIN*2)));
    GPIOB->MODER |=  ((1u<<(LED_RED_PIN*2)) |
                      (1u<<(LED_GREEN_PIN*2)) |
                      (1u<<(LED_BLUE_PIN*2)));
    GPIOB->ODR &= ~((1u<<LED_RED_PIN) |
                    (1u<<LED_GREEN_PIN) |
                    (1u<<LED_BLUE_PIN));
}

static void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3u<<(BUTTON_PIN*2));
    GPIOA->PUPDR &= ~(3u<<(BUTTON_PIN*2));
    GPIOA->PUPDR |=  (1u<<(BUTTON_PIN*2)); /* pull-up */
}

static void led_on(uint8_t pin) {
    GPIOB->ODR |= (1u<<pin);
}

static void led_off(uint8_t pin) {
    GPIOB->ODR &= ~(1u<<pin);
}

static void all_leds_off(void) {
    GPIOB->ODR &= ~((1u<<LED_RED_PIN) |
                    (1u<<LED_GREEN_PIN) |
                    (1u<<LED_BLUE_PIN));
}

static uint8_t button_read(void) {
    return (GPIOA->IDR & (1u<<BUTTON_PIN)) ? 1u : 0u;
}

/* ==== display ==== */

static void update_display(void) {
    char buf[8];

    st7789_fill_screen_dma(COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%02lu", (unsigned long)total_events);
    st7789_draw_text_5x7(80, 100, buf, COLOR_WHITE, 8, 0, 0);
}

/* ==== LED timing (300 ms) ==== */

static void update_leds(void) {
    TickType_t now = xTaskGetTickCount();
    const TickType_t led_on_time = pdMS_TO_TICKS(300);

    if (led_red_timer && (now - led_red_timer >= led_on_time)) {
        led_off(LED_RED_PIN);
        led_red_timer = 0;
    }
    if (led_green_timer && (now - led_green_timer >= led_on_time)) {
        led_off(LED_GREEN_PIN);
        led_green_timer = 0;
    }
    if (led_blue_timer && (now - led_blue_timer >= led_on_time)) {
        led_off(LED_BLUE_PIN);
        led_blue_timer = 0;
    }
}

/* ==== event detection ==== */

static void detect_events(const mpu6050_raw_t *data) {
    static uint8_t brake_detected = 0;
    static uint8_t crash_detected = 0;
    static uint8_t curve_detected = 0;

    if (data->ax < ACCEL_BRAKE_THRESHOLD && !brake_detected) {
        total_events++;
        brake_detected = 1;
        led_on(LED_RED_PIN);
        led_red_timer = xTaskGetTickCount();
    } else if (data->ax > ACCEL_BRAKE_THRESHOLD) {
        brake_detected = 0;
    }

    int16_t total_accel = data->ax;
    if (data->ay < 0) total_accel -= data->ay;
    else total_accel += data->ay;
    if (data->az < 0) total_accel -= data->az;
    else total_accel += data->az;

    if (total_accel > ACCEL_CRASH_THRESHOLD && !crash_detected) {
        total_events++;
        crash_detected = 1;
        led_on(LED_GREEN_PIN);
        led_green_timer = xTaskGetTickCount();
    } else if (total_accel < ACCEL_CRASH_THRESHOLD/2) {
        crash_detected = 0;
    }

    if ((data->gz > GYRO_CURVE_THRESHOLD ||
         data->gz < -GYRO_CURVE_THRESHOLD) && !curve_detected) {
        total_events++;
        curve_detected = 1;
        led_on(LED_BLUE_PIN);
        led_blue_timer = xTaskGetTickCount();
    } else if (data->gz < GYRO_CURVE_THRESHOLD/2 &&
               data->gz > -GYRO_CURVE_THRESHOLD/2) {
        curve_detected = 0;
    }
}

/* ==== tasks ==== */

/* Task de telemetria: única que envia pelo HC-12 */
static void TelemetryTask(void *arg) {
    (void)arg;
    uint32_t tx_count = 0;

    for (;;) {
        mpu6050_raw_t r;

        if (mpu6050_read_all(&r) == 0) {
            char buf[80];

            snprintf(buf, sizeof(buf),
                     "[CAMARADAS DO EDU]: %d, %d, %d, %d, %d, %d\n",
                     r.ax, r.ay, r.az, r.gx, r.gy, r.gz);

            hc12_send_string(buf);

            tx_count++;
            printf("TX[%lu]: %s",
                   (unsigned long)tx_count, buf);

            (void)xQueueSend(mpuQueue, &r, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void EventTask(void *arg) {
    (void)arg;
    mpu6050_raw_t r;

    for (;;) {
        if (xQueueReceive(mpuQueue, &r, pdMS_TO_TICKS(20)) == pdPASS) {
            detect_events(&r);
        }
        update_leds();
    }
}

static void DisplayTask(void *arg) {
    (void)arg;
    uint32_t last_events = (uint32_t)-1;

    for (;;) {
        uint32_t current = total_events;
        if (current != last_events) {
            update_display();
            last_events = current;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void ButtonTask(void *arg) {
    (void)arg;

    for (;;) {
        uint8_t button_curr = button_read();
        if (button_curr && !button_prev) {
            taskENTER_CRITICAL();
            total_events = 0;
            all_leds_off();
            led_red_timer   = 0;
            led_green_timer = 0;
            led_blue_timer  = 0;
            taskEXIT_CRITICAL();

            printf("Counter reset\n");
        }
        button_prev = button_curr;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ==== main ==== */

int main(void) {
    SystemCoreClockUpdate();   // garante SystemCoreClock correto

    leds_init();
    button_init();

    serial_stdio_init(115200);
    printf("=== SERVER DEBUG ===\n");

    hc12_init(9600);
    printf("HC12 initialized\n");

    led_on(LED_RED_PIN);
    busy_delay_ms(200);
    led_off(LED_RED_PIN);

    st7789_init();
    st7789_fill_screen_dma(COLOR_BLACK);
    busy_delay_ms(200);
    st7789_set_speed_div(2);
    printf("Display initialized\n");

    i2c1_init_100k(50000000u);

    if (mpu6050_init() < 0) {
        printf("MPU6050 INIT FAILED\n");
        st7789_fill_screen_dma(COLOR_RED);
        st7789_draw_text_5x7(10, 100, "MPU INIT ERROR", COLOR_WHITE, 2, 0, 0);
        for (;;) {
            led_on(LED_RED_PIN);
            busy_delay_ms(150);
            led_off(LED_RED_PIN);
            busy_delay_ms(150);
        }
    }

    printf("MPU6050 initialized\n");
    update_display();
    hc12_send_string("SYSTEM READY\n");
    printf("System ready, starting scheduler\n");

    mpuQueue = xQueueCreate(8, sizeof(mpu6050_raw_t));
    if (mpuQueue == NULL) {
        printf("Queue create failed\n");
        for (;;) {
        }
    }

    xTaskCreate(TelemetryTask, "MPU_TX",   256, NULL, 3, NULL);
    xTaskCreate(EventTask,     "EVENTS",   256, NULL, 2, NULL);
    xTaskCreate(DisplayTask,   "DISPLAY",  256, NULL, 1, NULL);
    xTaskCreate(ButtonTask,    "BUTTON",   128, NULL, 1, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
}
