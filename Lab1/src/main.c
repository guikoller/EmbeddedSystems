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
uint32_t last_tick_ms = 0;
uint32_t last_tick_s = 0;
uint32_t color_tick_ms = 1000;

uint16_t colors[] = {
    C_RED, C_GREEN, C_BLUE, C_YELL, C_CYAN, C_WHITE
};


static inline int  uart_rx_ready(void) { return (USART1->SR & USART_SR_RXNE) != 0; }
static inline char uart_getc(void)     { return (char)USART1->DR; }

static void clean_header(void){
    st7789_fill_rect_dma(0, 0, LCD_W, HEADER_HEIGHT, C_BLACK);
}

static void draw_header(uint32_t uptime_ms){
    if (uptime_ms - last_tick_ms >= color_tick_ms) {
        static uint8_t i = 0;

        clean_header();

        uint32_t seconds = uptime_ms / 1000u;
        char string_to_value[24];
        snprintf(string_to_value, sizeof(string_to_value), "Uptime: %lu s", seconds);
        st7789_draw_text_5x7(PADDING, PADDING, string_to_value, C_WHITE, 2, 0, 0);

        uint16_t x = (uint16_t)(LCD_W - SQUARE_SIZE - PADDING);
        uint16_t y = (uint16_t)PADDING;
        uint16_t w = (uint16_t)SQUARE_SIZE;
        uint16_t h = (uint16_t)SQUARE_SIZE;
        st7789_fill_rect_dma(x, y, w, h, colors[i]);

        i = (uint8_t)((i + 1u) % (sizeof(colors)/sizeof(colors[0])));
        last_tick_ms = uptime_ms;
        last_tick_s  = seconds;
    }
}


static void demo_clear(void){
    printf("[demo 1] Clear screen (black)\r\n");
    st7789_fill_screen_dma(C_BLACK);
    st7789_draw_text_5x7(20, 20, "CLEAR BLACK", C_WHITE, 2, 0, 0);
}

static void demo_pixels(void){
    printf("[demo 2] Random pixels\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<2000;i++){
        int x = rand() % LCD_W;
        int y = rand() % LCD_H;
        uint16_t c = (uint16_t)rand();
        st7789_draw_pixel(x,y,c);
    }
}

static void demo_hlines(void){
    printf("[demo 3] Horizontal lines\r\n");
    for (int y=0;y<LCD_H;y+=10){
        uint16_t c = (uint16_t)rand();
        st7789_draw_hline(0,y,LCD_W,c);
    }
}

static void demo_vlines(void){
    printf("[demo 4] Vertical lines\r\n");
    for (int x=0;x<LCD_W;x+=10){
        uint16_t c = (uint16_t)rand();
        st7789_draw_vline(x,0,LCD_H,c);
    }
}

static void demo_rects(void){
    printf("[demo 5] Random rectangles (outline)\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<10;i++){
        int x = rand()%200;
        int y = rand()%200;
        int w = 20+rand()%40;
        int h = 20+rand()%40;
        uint16_t c = (uint16_t)rand();
        st7789_draw_rect(x,y,w,h,c);
    }
}

static void demo_fills(void){
    printf("[demo 6] Filled rectangles (DMA)\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<5;i++){
        int x = rand()%200;
        int y = rand()%200;
        int w = 20+rand()%40;
        int h = 20+rand()%40;
        uint16_t c = (uint16_t)rand();
        printf("  rect #%d: x=%d y=%d w=%d h=%d color=0x%04X\r\n",
               i, x,y,w,h,c);
        st7789_fill_rect_dma(x,y,w,h,c);
        delay_ms(200);
    }
}

static void demo_circles(void){
    printf("[demo 7] Random circles\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<8;i++){
        int x = rand()%LCD_W;
        int y = rand()%LCD_H;
        int r = 10+rand()%30;
        uint16_t c = (uint16_t)rand();
        printf("  circle #%d: cx=%d cy=%d r=%d color=0x%04X\r\n",
               i, x,y,r,c);
        st7789_draw_circle(x,y,r,c);
    }
}

static void demo_fillcircles(void){
    printf("[demo 8] Filled circles\r\n");
    st7789_fill_screen(C_BLACK);
    for (int i=0;i<6;i++){
        int x = rand()%LCD_W;
        int y = rand()%LCD_H;
        int r = 10+rand()%40;
        uint16_t c = (uint16_t)rand();
        printf("  fillcirc #%d: cx=%d cy=%d r=%d color=0x%04X\r\n",
               i, x,y,r,c);
        st7789_fill_circle(x,y,r,c);
    }
}

static void demo_text(void){
    printf("[demo 9] Text demo\r\n");
    st7789_fill_screen(C_BLACK);
    st7789_draw_text_5x7(10, 40, "UTFPR - SIS.EMBARCADOS", C_YELL, 2, 0, 0);
    st7789_draw_text_5x7(10, 80, "MENU DEMO", C_CYAN, 2, 0, 0);
}

int main(void){
    delay_init();
    serial_stdio_init(115200);

    st7789_init();
    st7789_set_speed_div(0);

    printf("\r\n=== MENU ST7789 ===\r\n");
    printf("Digite 1..9 e pressione Enter:\r\n");
    printf("1: Clear\n2: Pixels\n3: Hlines\n4: Vlines\n5: Rects\n");
    printf("6: Fills\n7: Circles\n8: FillCircles\n9: Text\r\n");

    st7789_fill_screen(C_BLACK);
    draw_header(millis());

    for(;;){
        draw_header(millis());

        if (uart_rx_ready()){
            char c = uart_getc();
            switch(c){
                case '1': demo_clear(); break;
                case '2': demo_pixels(); break;
                case '3': demo_hlines(); break;
                case '4': demo_vlines(); break;
                case '5': demo_rects(); break;
                case '6': demo_fills(); break;
                case '7': demo_circles(); break;
                case '8': demo_fillcircles(); break;
                case '9': demo_text(); break;
                default: break;
            }
        }
    }
}