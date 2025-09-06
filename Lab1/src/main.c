#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "serial.h"
#include "st7789.h"
#include "board.h"
#include "main.h" 

#define HEADER_HEIGHT 32
#define PADDING 2
#define SQUARE_SIZE (HEADER_HEIGHT - 2*PADDING)

static const uint16_t colors[] = {
    C_RED, C_GREEN, C_BLUE, C_YELL, C_CYAN, C_WHITE
};

#define COLORS_LEN (sizeof(colors)/sizeof(colors[0]))

static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }

uint32_t last_tick_ms = 0;
uint32_t last_tick_s = 0;
uint32_t color_tick_ms = 1000;

volatile uint32_t ball_delay = 20;

void increase_ball_speed(void) {
    if (ball_delay > 2) ball_delay -= 2;
}

void decrease_ball_speed(void) {
    if (ball_delay < 100) ball_delay += 2;
}
static void clean_header(void){
    st7789_fill_rect_dma(0, 0, LCD_W, HEADER_HEIGHT, C_BLACK);
}

static void draw_header(uint32_t uptime_ms){
    if (uptime_ms - last_tick_ms >= color_tick_ms) {
        clean_header();

        uint32_t seconds = uptime_ms / 1000u;
        char value_to_string[24];
        snprintf(value_to_string, sizeof(value_to_string), "Uptime: %lu s", seconds);
        st7789_draw_text_5x7(0, 0, value_to_string, C_WHITE, 2, 0, 0);

        static uint8_t i = 0;
        draw_header_square(i);
        i = (uint8_t)((i + 1u) % COLORS_LEN);

        last_tick_ms = uptime_ms;
        last_tick_s  = seconds;
    }
}

static void draw_header_square(uint8_t i){
    uint16_t x = (uint16_t)(LCD_W - SQUARE_SIZE - PADDING);
    uint16_t y = (uint16_t)PADDING;
    uint16_t w = (uint16_t)SQUARE_SIZE;
    uint16_t h = (uint16_t)SQUARE_SIZE;
    st7789_fill_rect_dma(x, y, w, h, colors[i]);
}

void animate_bouncing_circle(void) {
    static int x = 50, y = 160, r = 20, dx = 4;
    static uint32_t last_update = 0;
    static uint32_t last_log = 0;
    uint32_t now = millis();
    if (now - last_update < ball_delay) return;
    last_update = now;

    int old_x = x;

    st7789_fill_circle(old_x, y, r, C_BLACK);

    x += dx;
    if (x + r >= LCD_W) 
    {
        x = LCD_W - r;
        dx = -dx;
    } 
    else if (x - r <= 0) 
    {
        x = r;
        dx = -dx;
    }

    st7789_fill_circle(x, y, r, C_GREEN);

    if (now - last_log >= 200) {
        printf("[LOG] Uptime: %lu ms | Ball X: %d\r\n", now, x);
        last_log = now;
    }
}

int main(void){
    delay_init();
    serial_stdio_init(115200);

    st7789_init();
    st7789_set_speed_div(0);

    printf("\r\n=== MENU ST7789 ===\r\n");
    printf("LAB 01 - Display\r\n");
    printf("1: Aumentar Velocidade\n");
    printf("2: Diminuir Velocidade\n");

    st7789_fill_screen(C_BLACK);
    draw_header(millis());

    for(;;){
        draw_header(millis());
        animate_bouncing_circle();
        if (uart_rx_ready()){
            char c = uart_getc();
            switch(c){
                case '1': increase_ball_speed(); break;
                case '2': decrease_ball_speed(); break;
                default: break;
            }
        }
    }
}