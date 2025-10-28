#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include "serial_stdio.h"
#include "mpu6050.h"
#include "st7789.h"
#include "delay.h"

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

static uint32_t total_events = 0;
static uint8_t button_prev = 0;

static uint32_t led_red_timer = 0;
static uint32_t led_green_timer = 0;
static uint32_t led_blue_timer = 0;

static void hc12_init(uint32_t baudrate) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    
    GPIOA->MODER &= ~((3u<<(2*2)) | (3u<<(3*2)));
    GPIOA->MODER |=  ((2u<<(2*2)) | (2u<<(3*2)));
    GPIOA->AFR[0] &= ~((0xFu<<(2*4)) | (0xFu<<(3*4)));
    GPIOA->AFR[0] |=  ((7u<<(2*4)) | (7u<<(3*4)));
    
    uint32_t apb1 = SystemCoreClock / 2;
    USART2->BRR = apb1 / baudrate;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

static void hc12_send_char(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

static void hc12_send_string(const char* s) {
    while (*s) {
        hc12_send_char(*s++);
    }
}

static void leds_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~((3u<<(LED_RED_PIN*2)) | (3u<<(LED_GREEN_PIN*2)) | (3u<<(LED_BLUE_PIN*2)));
    GPIOB->MODER |=  ((1u<<(LED_RED_PIN*2)) | (1u<<(LED_GREEN_PIN*2)) | (1u<<(LED_BLUE_PIN*2)));
    GPIOB->ODR &= ~((1u<<LED_RED_PIN) | (1u<<LED_GREEN_PIN) | (1u<<LED_BLUE_PIN));
}

static void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3u<<(BUTTON_PIN*2));
    GPIOA->PUPDR &= ~(3u<<(BUTTON_PIN*2));
    GPIOA->PUPDR |=  (1u<<(BUTTON_PIN*2));
}

static void led_on(uint8_t pin) {
    GPIOB->ODR |= (1u<<pin);
}

static void led_off(uint8_t pin) {
    GPIOB->ODR &= ~(1u<<pin);
}

static void all_leds_off(void) {
    GPIOB->ODR &= ~((1u<<LED_RED_PIN) | (1u<<LED_GREEN_PIN) | (1u<<LED_BLUE_PIN));
}

static uint8_t button_read(void) {
    return (GPIOA->IDR & (1u<<BUTTON_PIN)) ? 1 : 0;
}

static void update_display(void) {
    char buf[8];
    
    st7789_fill_screen_dma(COLOR_BLACK);
    
    snprintf(buf, sizeof(buf), "%02lu", total_events);
    st7789_draw_text_5x7(80, 100, buf, COLOR_WHITE, 8, 0, 0);
}

static void update_leds(void) {
    uint32_t now = millis();
    
    if (led_red_timer && (now - led_red_timer >= 3000)) {
        led_off(LED_RED_PIN);
        led_red_timer = 0;
    }
    
    if (led_green_timer && (now - led_green_timer >= 3000)) {
        led_off(LED_GREEN_PIN);
        led_green_timer = 0;
    }
    
    if (led_blue_timer && (now - led_blue_timer >= 3000)) {
        led_off(LED_BLUE_PIN);
        led_blue_timer = 0;
    }
}

static void detect_events(mpu6050_raw_t *data) {
    static uint8_t brake_detected = 0;
    static uint8_t crash_detected = 0;
    static uint8_t curve_detected = 0;
    
    if (data->ax < ACCEL_BRAKE_THRESHOLD && !brake_detected) {
        total_events++;
        brake_detected = 1;
        led_on(LED_RED_PIN);
        led_red_timer = millis();
        update_display();
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
        led_green_timer = millis();
        update_display();
    } else if (total_accel < ACCEL_CRASH_THRESHOLD/2) {
        crash_detected = 0;
    }
    
    if ((data->gz > GYRO_CURVE_THRESHOLD || data->gz < -GYRO_CURVE_THRESHOLD) && !curve_detected) {
        total_events++;
        curve_detected = 1;
        led_on(LED_BLUE_PIN);
        led_blue_timer = millis();
        update_display();
    } else if (data->gz < GYRO_CURVE_THRESHOLD/2 && data->gz > -GYRO_CURVE_THRESHOLD/2) {
        curve_detected = 0;
    }
}

int main(void) {
    SystemCoreClockUpdate();
    delay_init();
    
    leds_init();
    button_init();
    
    serial_stdio_init(115200);
    printf("=== SERVER DEBUG ===\n");
    
    hc12_init(9600);
    printf("HC12 initialized\n");
    
    led_on(LED_RED_PIN);
    delay_ms(500);
    led_off(LED_RED_PIN);
    
    st7789_init();
    st7789_fill_screen_dma(COLOR_BLACK);
    delay_ms(500);
    st7789_set_speed_div(2);
    printf("Display initialized\n");
    
    i2c1_init_100k(50000000u);
    
    if (mpu6050_init() < 0) {
        printf("MPU6050 INIT FAILED\n");
        st7789_fill_screen_dma(COLOR_RED);
        st7789_draw_text_5x7(10, 100, "MPU INIT ERROR", COLOR_WHITE, 2, 0, 0);
        for (;;) {
            led_on(LED_RED_PIN);
            delay_ms(150);
            led_off(LED_RED_PIN);
            delay_ms(150);
        }
    }
    
    printf("MPU6050 initialized\n");
    update_display();
    hc12_send_string("SYSTEM READY\n");
    printf("System ready, starting loop\n");
    
    uint32_t tx_count = 0;
    
    for (;;) {
        mpu6050_raw_t r;
        
        uint8_t button_curr = button_read();
        if (button_curr && !button_prev) {
            total_events = 0;
            update_display();
            all_leds_off();
            led_red_timer = 0;
            led_green_timer = 0;
            led_blue_timer = 0;
            printf("Counter reset\n");
            delay_ms(200);
        }
        button_prev = button_curr;
        
        if (mpu6050_read_all(&r) == 0) {
            detect_events(&r);
            
            char buf[80];
            snprintf(buf, sizeof(buf), "[CAMARADAS DO EDU]: %d, %d, %d, %d, %d, %d\n",
                     r.ax, r.ay, r.az, r.gx, r.gy, r.gz);
            
            hc12_send_string(buf);
            tx_count++;
            
            printf("TX[%lu]: %s", tx_count, buf);
        }
        
        update_leds();
        delay_ms(50);
    }
}