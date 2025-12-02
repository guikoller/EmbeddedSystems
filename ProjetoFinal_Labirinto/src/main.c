/*
 * Projeto Final - Simulador Digital de Labirinto
 * Equipe: Alfons, Mateus, Guilherme
 * Prof. Eduardo Nunes Dos Santos
 * ELF74 - Sistemas Embarcados
 * 
 * REQUISITOS IMPLEMENTADOS:
 *   Mínimo 3 threads/tarefas (5 tarefas criadas)
 *   Máquina de estados (GameLogic_Task)
 *   Objetos de sincronização (Queue, Mutex, Semáforo)
 *   Temporização determinística (vTaskDelayUntil, TickCount)
 *   Relógio hh:mm:ss no display
 */

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "serial_stdio.h"
#include "mpu6050.h"
#include "st7789.h"
#include "delay_rtos.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* ==== FreeRTOS Hooks ==== */

void vAssertCalled(const char *file, int line) {
    (void)file;
    (void)line;
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

/* ==== Defines e Constantes ==== */

#define BUTTON_PIN      0

// Cores
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_WHITE     0xFFFF
#define COLOR_BLACK     0x0000
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_GRAY      0x8410
#define COLOR_ORANGE    0xFD20

// Dimensões do labirinto
#define MAZE_WIDTH      16
#define MAZE_HEIGHT     16
#define CELL_SIZE       13      // pixels por célula (Reduzido de 14 para 13 para caber na tela)
#define MAZE_OFFSET_X   16      // Centralizado horizontalmente
#define MAZE_OFFSET_Y   30      // Espaço para o relógio no topo

// Tipo de célula
#define CELL_EMPTY      0
#define CELL_WALL       1
#define CELL_HOLE       2
#define CELL_GOAL       3

// Física
#define GRAVITY         200.0f   // aceleração da gravidade (pixels/s²)
#define BALL_RADIUS     4        // raio da bolinha em pixels
#define MAX_VELOCITY    40.0f    // velocidade máxima (pixels/s)
#define TILT_SCALE      0.0005f  // escala de conversão acelerômetro -> aceleração

// Jogo
#define MAX_LIVES       3
#define WIN_TIME_BONUS  1000

/* ==== Estruturas ==== */

typedef struct {
    float x, y;          // posição (pixels)
    float vx, vy;        // velocidade (pixels/s)
    float ax, ay;        // aceleração (pixels/s²)
} Ball;

typedef struct {
    uint8_t cells[MAZE_HEIGHT][MAZE_WIDTH];
    uint8_t start_x, start_y;
    uint8_t goal_x, goal_y;
} Maze;

/* REQUISITO: Máquina de Estados */
typedef enum {
    GAME_INIT,           // Estado inicial
    GAME_SELECT_MAP,     // Seleção de Mapa
    GAME_READY,          // Pronto para começar
    GAME_PLAYING,        // Jogando
    GAME_PAUSED,         // Pausado
    GAME_LOST_LIFE,      // Perdeu uma vida
    GAME_WON,            // Venceu
    GAME_OVER            // Game Over
} GameState;

/* Estrutura de relógio (hh:mm:ss) */
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} ClockTime;

/* ==== Variáveis Globais ==== */

static Ball ball;
static Maze maze;
static volatile GameState game_state = GAME_INIT;
static uint8_t lives = MAX_LIVES;
static uint32_t game_time_ms = 0;
static uint32_t best_time_ms = 0xFFFFFFFF;
static int selected_map_idx = 0;

/* REQUISITO: Relógio hh:mm:ss */
static ClockTime system_clock = {0, 0, 0};
static volatile uint8_t clock_updated = 0;

/* REQUISITO: Objetos de Sincronização */
static QueueHandle_t imu_queue = NULL;           // Queue para dados IMU
static SemaphoreHandle_t display_mutex = NULL;   // Mutex para acesso ao display
static SemaphoreHandle_t game_state_sem = NULL;  // Semáforo binário para estado do jogo
static TimerHandle_t clock_timer = NULL;         // Software timer para relógio

static volatile uint8_t button_pressed = 0;
static uint8_t button_prev = 0;

// Offsets de calibração do MPU6050
static int16_t accel_offset_x = 0;
static int16_t accel_offset_y = 0;

/* ==== Labirinto ==== */

// Mapa 1: Clássico
static const uint8_t maze_map1[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,2,0,0,0,1,0,1,0,1},
    {1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
    {1,0,0,2,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,2,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Mapa 2: ZigZag
static const uint8_t maze_map2[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,2,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Mapa 3: Pilares
static const uint8_t maze_map3[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static const uint8_t (*maps[])[MAZE_WIDTH] = { maze_map1, maze_map2, maze_map3 };
static const char *map_names[] = { "CLASSIC", "ZIGZAG", "PILLARS" };

static void maze_init(void) {
    memcpy(maze.cells, maps[selected_map_idx], sizeof(maze_map1));
    maze.start_x = 1;
    maze.start_y = 1;
    maze.goal_x = 14;
    maze.goal_y = 14;
}

static uint8_t maze_get_cell(int x, int y) {
    if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT) {
        return CELL_WALL;
    }
    return maze.cells[y][x];
}

/* ==== Física da Bolinha ==== */

static void ball_init(void) {
    ball.x = (maze.start_x * CELL_SIZE) + (CELL_SIZE / 2.0f) + MAZE_OFFSET_X;
    ball.y = (maze.start_y * CELL_SIZE) + (CELL_SIZE / 2.0f) + MAZE_OFFSET_Y;
    ball.vx = 0.0f;
    ball.vy = 0.0f;
    ball.ax = 0.0f;
    ball.ay = 0.0f;
}

static void ball_update_physics(float dt, float tilt_x, float tilt_y) {
    // Aplicar aceleração baseada na inclinação (MPU6050)
    ball.ax = tilt_x * GRAVITY * TILT_SCALE;
    ball.ay = tilt_y * GRAVITY * TILT_SCALE;
    
    // Atualizar velocidade
    ball.vx += ball.ax * dt;
    ball.vy += ball.ay * dt;
    
    
    // Limitar velocidade máxima
    float speed = sqrtf(ball.vx * ball.vx + ball.vy * ball.vy);
    if (speed > MAX_VELOCITY) {
        ball.vx = (ball.vx / speed) * MAX_VELOCITY;
        ball.vy = (ball.vy / speed) * MAX_VELOCITY;
    }
    
    // Calcular nova posição
    float new_x = ball.x + ball.vx * dt;
    float new_y = ball.y + ball.vy * dt;
    
    // Verificar colisão com paredes (8 pontos ao redor da bola)
    uint8_t collision = 0;
    for (int angle = 0; angle < 360; angle += 45) {
        float rad = angle * 3.14159f / 180.0f;
        float check_x = new_x + cosf(rad) * BALL_RADIUS;
        float check_y = new_y + sinf(rad) * BALL_RADIUS;
        
        int cell_x = (int)((check_x - MAZE_OFFSET_X) / CELL_SIZE);
        int cell_y = (int)((check_y - MAZE_OFFSET_Y) / CELL_SIZE);
        
        if (maze_get_cell(cell_x, cell_y) == CELL_WALL) {
            collision = 1;
            break;
        }
    }
    
    if (!collision) {
        ball.x = new_x;
        ball.y = new_y;
    } else {
        // Colisão: inverter velocidade e parar
        ball.vx *= -0.3f;
        ball.vy *= -0.3f;
    }
}

static uint8_t ball_check_hole(void) {
    int cell_x = (int)((ball.x - MAZE_OFFSET_X) / CELL_SIZE);
    int cell_y = (int)((ball.y - MAZE_OFFSET_Y) / CELL_SIZE);
    return (maze_get_cell(cell_x, cell_y) == CELL_HOLE);
}

static uint8_t ball_check_goal(void) {
    int cell_x = (int)((ball.x - MAZE_OFFSET_X) / CELL_SIZE);
    int cell_y = (int)((ball.y - MAZE_OFFSET_Y) / CELL_SIZE);
    return (maze_get_cell(cell_x, cell_y) == CELL_GOAL);
}

/* ==== Relógio (hh:mm:ss) ==== */

/* Callback do Timer: atualiza relógio a cada 1 segundo */
static void vClockTimerCallback(TimerHandle_t xTimer) {
    (void)xTimer;
    
    system_clock.seconds++;
    if (system_clock.seconds >= 60) {
        system_clock.seconds = 0;
        system_clock.minutes++;
        if (system_clock.minutes >= 60) {
            system_clock.minutes = 0;
            system_clock.hours++;
            if (system_clock.hours >= 24) {
                system_clock.hours = 0;
            }
        }
    }
    
    clock_updated = 1;
}

/* ==== Renderização ==== */

static void render_map_selector(void) {
    st7789_fill_screen_dma(COLOR_BLACK);
    st7789_draw_text_5x7(60, 40, "SELECT MAP", COLOR_WHITE, 2, 0, 0);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "< %s >", map_names[selected_map_idx]);
    
    // Centralizar texto (aprox)
    int len = strlen(buf);
    int x = (240 - (len * 12)) / 2; // 12px por char (escala 2)
    if (x < 0) x = 0;
    
    st7789_draw_text_5x7(x, 100, buf, COLOR_YELLOW, 2, 0, 0);
    
    st7789_draw_text_5x7(80, 160, "Tilt DOWN", COLOR_CYAN, 1, 0, 0);
    st7789_draw_text_5x7(80, 175, "to confirm", COLOR_CYAN, 1, 0, 0);
}

static void render_clock(void) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", 
             system_clock.hours, system_clock.minutes, system_clock.seconds);
    st7789_draw_text_5x7(5, 12, buf, COLOR_YELLOW, 1, 1, COLOR_BLACK);
}

static void render_maze(void) {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            uint16_t color;
            uint8_t cell = maze.cells[y][x];
            
            switch (cell) {
                case CELL_WALL:  color = COLOR_GRAY;    break;
                case CELL_HOLE:  color = COLOR_BLACK;   break;
                case CELL_GOAL:  color = COLOR_GREEN;   break;
                default:         color = COLOR_WHITE;   break;
            }
            
            int px = x * CELL_SIZE + MAZE_OFFSET_X;
            int py = y * CELL_SIZE + MAZE_OFFSET_Y;
            st7789_fill_rect_dma(px, py, CELL_SIZE, CELL_SIZE, color);
        }
    }
}

static void render_ball(void) {
    int x = (int)ball.x;
    int y = (int)ball.y;
    
    // Desenhar círculo simples (quadrado por enquanto)
    st7789_fill_rect_dma(x - BALL_RADIUS, y - BALL_RADIUS, 
                         BALL_RADIUS * 2, BALL_RADIUS * 2, COLOR_RED);
}

static void render_hud(void) {
    char buf[32];
    
    // Relógio (REQUISITO: hh:mm:ss)
    render_clock();
    
    // Vidas
    snprintf(buf, sizeof(buf), "LIVES:%d", lives);
    st7789_draw_text_5x7(90, 12, buf, COLOR_WHITE, 1, 1, COLOR_BLACK);
    
    // Tempo do jogo
    uint32_t sec = game_time_ms / 1000;
    uint32_t ms = game_time_ms % 1000;
    snprintf(buf, sizeof(buf), "T:%02lu.%03lu", (unsigned long)sec, (unsigned long)ms);
    st7789_draw_text_5x7(165, 12, buf, COLOR_CYAN, 1, 1, COLOR_BLACK);
}

static void render_game_over(void) {
    st7789_fill_screen_dma(COLOR_BLACK);
    st7789_draw_text_5x7(50, 100, "GAME OVER", COLOR_RED, 2, 0, 0);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "TIME: %lu.%03lu s", 
             (unsigned long)(game_time_ms/1000), 
             (unsigned long)(game_time_ms%1000));
    st7789_draw_text_5x7(40, 130, buf, COLOR_WHITE, 1, 0, 0);
    
    st7789_draw_text_5x7(30, 160, "Press button to restart", COLOR_YELLOW, 1, 0, 0);
}

static void render_win(void) {
    st7789_fill_screen_dma(COLOR_BLACK);
    st7789_draw_text_5x7(60, 90, "YOU WIN!", COLOR_GREEN, 2, 0, 0);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "TIME: %lu.%03lu s", 
             (unsigned long)(game_time_ms/1000), 
             (unsigned long)(game_time_ms%1000));
    st7789_draw_text_5x7(40, 120, buf, COLOR_WHITE, 1, 0, 0);
    
    if (best_time_ms != 0xFFFFFFFF) {
        snprintf(buf, sizeof(buf), "BEST: %lu.%03lu s", 
                 (unsigned long)(best_time_ms/1000), 
                 (unsigned long)(best_time_ms%1000));
        st7789_draw_text_5x7(40, 135, buf, COLOR_YELLOW, 1, 0, 0);
    }
    
    st7789_draw_text_5x7(30, 165, "Press button to restart", COLOR_CYAN, 1, 0, 0);
}

/* ==== GPIO ==== */

static void button_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3u << (BUTTON_PIN * 2));
    GPIOA->PUPDR &= ~(3u << (BUTTON_PIN * 2));
    GPIOA->PUPDR |= (1u << (BUTTON_PIN * 2)); // pull-up
}

static uint8_t button_read(void) {
    return (GPIOA->IDR & (1u << BUTTON_PIN)) ? 1u : 0u;
}

/* ==== Tasks FreeRTOS - REQUISITO: Mínimo 3 tarefas ==== */

/* Task 1: Leitura do IMU (30Hz) - REQUISITO: Temporização determinística */
static void IMU_Task(void *arg) {
    (void)arg;
    mpu6050_raw_t imu_data;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(33); // ~30Hz
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        if (mpu6050_read_all(&imu_data) == 0) {
            // REQUISITO: Usar Queue para sincronização
            xQueueSend(imu_queue, &imu_data, 0);
        }
        
        // REQUISITO: Temporização determinística com vTaskDelayUntil
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Task 2: Lógica do Jogo (30Hz) - REQUISITO: Máquina de Estados */
static void GameLogic_Task(void *arg) {
    (void)arg;
    mpu6050_raw_t imu_data;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(33); // ~30Hz
    TickType_t last_physics_time;
    uint32_t start_ticks = 0;
    
    xLastWakeTime = xTaskGetTickCount();
    last_physics_time = xLastWakeTime;
    
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        float dt = (float)(now - last_physics_time) / configTICK_RATE_HZ;
        last_physics_time = now;
        
        /* REQUISITO: Máquina de Estados */
        switch (game_state) {
            case GAME_INIT:
                printf("[STATE] INIT -> SELECT_MAP\n");
                game_state = GAME_SELECT_MAP;
                break;

            case GAME_SELECT_MAP:
                if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
                    float tilt_x = (float)(imu_data.ax - accel_offset_x);
                    float tilt_y = (float)(imu_data.ay - accel_offset_y);
                    
                    // Screen X = -tilt_y (Direita/Esquerda)
                    // Screen Y = -tilt_x (Cima/Baixo)
                    float screen_x = -tilt_y;
                    float screen_y = -tilt_x;
                    
                    static uint32_t last_move_time = 0;
                    
                    // Navegação (Direita/Esquerda) com delay para evitar troca rápida
                    if (now - last_move_time > pdMS_TO_TICKS(400)) {
                        if (screen_x > 4000) { // Direita
                            selected_map_idx = (selected_map_idx + 1) % 3;
                            last_move_time = now;
                            printf("[MENU] Map: %d\n", selected_map_idx);
                        } else if (screen_x < -4000) { // Esquerda
                            selected_map_idx = (selected_map_idx + 2) % 3; // -1
                            last_move_time = now;
                            printf("[MENU] Map: %d\n", selected_map_idx);
                        }
                    }
                    
                    // Confirmação (Baixo)
                    if (screen_y > 5000) { // Baixo
                         printf("[STATE] SELECT_MAP -> READY (Map %d)\n", selected_map_idx);
                         maze_init();
                         ball_init();
                         lives = MAX_LIVES;
                         game_time_ms = 0;
                         game_state = GAME_READY;
                    }
                }
                break;
                
            case GAME_READY:
                printf("[STATE] READY -> PLAYING\n");
                start_ticks = xTaskGetTickCount();
                game_state = GAME_PLAYING;
                // REQUISITO: Sinalizar com semáforo
                xSemaphoreGive(game_state_sem);
                break;
                
            case GAME_PLAYING:
                // Atualizar tempo do jogo
                game_time_ms = (xTaskGetTickCount() - start_ticks) * portTICK_PERIOD_MS;
                
                // REQUISITO: Receber dados da Queue
                if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
                    // Atualizar física da bola
                    // Mapeamento: X do MPU -> X do Display (Horizontal)
                    //             Y do MPU -> Y do Display (Vertical)
                    // Subtraindo offsets de calibração
                    float tilt_x = (float)(imu_data.ax - accel_offset_x);
                    float tilt_y = (float)(imu_data.ay - accel_offset_y);
                    
                    // Mapeamento corrigido conforme log:
                    // "Virar pra baixo" gerou AX negativo (-14000).
                    // No display, Y aumenta para baixo. Então Display Y = -Sensor X.
                    // Assumindo rotação de 90 graus: Display X = Sensor Y.
                    // Invertendo X (Direita/Esquerda) conforme solicitado.
                    
                    ball_update_physics(dt, -tilt_y, -tilt_x);

                    // DEBUG: Imprimir valores do MPU a cada ~1 segundo (50 ciclos de 20ms)
                    static int debug_cnt = 0;
                    if (++debug_cnt >= 25) { // 25 * 20ms = 500ms
                        debug_cnt = 0;
                    }
                }
                
                // Verificar queda em buraco
                if (ball_check_hole()) {
                    lives--;
                    printf("[EVENT] Fell in hole! Lives: %d\n", lives);
                    
                    if (lives > 0) {
                        printf("[STATE] PLAYING -> LOST_LIFE\n");
                        game_state = GAME_LOST_LIFE;
                    } else {
                        printf("[STATE] PLAYING -> GAME_OVER\n");
                        game_state = GAME_OVER;
                    }
                }
                
                // Verificar chegada ao objetivo
                if (ball_check_goal()) {
                    printf("[STATE] PLAYING -> GAME_WON\n");
                    game_state = GAME_WON;
                    if (game_time_ms < best_time_ms) {
                        best_time_ms = game_time_ms;
                    }
                    printf("[EVENT] You Won! Time: %lu ms\n", (unsigned long)game_time_ms);
                }
                break;
                
            case GAME_LOST_LIFE:
                // Resetar posição da bola
                ball_init();
                vTaskDelay(pdMS_TO_TICKS(1000)); // Pausa de 1 segundo
                printf("[STATE] LOST_LIFE -> PLAYING\n");
                game_state = GAME_PLAYING;
                break;
                
            case GAME_PAUSED:
                // Estado de pausa (pode ser usado futuramente)
                vTaskDelay(pdMS_TO_TICKS(100));
                break;
                
            case GAME_WON:
            case GAME_OVER:
                // Aguardar botão para reiniciar
                if (button_pressed) {
                    button_pressed = 0;
                    printf("[STATE] %s -> INIT\n", 
                           game_state == GAME_WON ? "GAME_WON" : "GAME_OVER");
                    game_state = GAME_INIT;
                }
                break;
                
            default:
                break;
        }
        
        // REQUISITO: Temporização determinística com vTaskDelayUntil
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Task 3: Renderização do Display (10Hz) - REQUISITO: Mutex */
static void Display_Task(void *arg) {
    (void)arg;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100); // 10Hz
    
    xLastWakeTime = xTaskGetTickCount();
    
    static GameState last_drawn_state = -1; // Estado inválido para forçar primeiro desenho
    static int last_drawn_map_idx = -1;

    for (;;) {
        // REQUISITO: Usar Mutex para acesso ao display
        if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            int state_changed = (game_state != last_drawn_state);
            int map_changed = (selected_map_idx != last_drawn_map_idx);
            
            last_drawn_state = game_state;
            last_drawn_map_idx = selected_map_idx;

            switch (game_state) {
                case GAME_SELECT_MAP:
                    if (state_changed || map_changed) {
                        render_map_selector();
                    }
                    break;

                case GAME_READY:
                case GAME_PLAYING:
                case GAME_LOST_LIFE:
                    render_maze();
                    render_ball();
                    render_hud();
                    break;
                    
                case GAME_WON:
                    // Só redesenha a tela de vitória se o estado mudou (evita flicker)
                    if (state_changed) render_win();
                    render_clock(); // Relógio continua atualizando
                    break;
                    
                case GAME_OVER:
                    // Só redesenha a tela de Game Over se o estado mudou (evita flicker)
                    if (state_changed) render_game_over();
                    render_clock(); // Relógio continua atualizando
                    break;
                    
                default:
                    break;
            }
            
            xSemaphoreGive(display_mutex);
        }
        
        // REQUISITO: Temporização determinística com vTaskDelayUntil
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Task 4: Leitura do Botão (10Hz) */
static void Button_Task(void *arg) {
    (void)arg;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(100); // 10Hz
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        uint8_t button_curr = button_read();
        
        // Detecção de borda de subida (botão pressionado)
        if (button_curr && !button_prev) {
            button_pressed = 1;
            printf("[INPUT] Button pressed\n");
        }
        
        button_prev = button_curr;
        
        // REQUISITO: Temporização determinística com vTaskDelayUntil
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Task 5: Atualização do Relógio no Display (1Hz) - REQUISITO */
static void ClockDisplay_Task(void *arg) {
    (void)arg;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(1000); // 1Hz (1 segundo)
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        // Aguardar até que o relógio seja atualizado pelo timer
        if (clock_updated) {
            clock_updated = 0;
            
            // REQUISITO: Usar Mutex para acesso ao display
            if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                render_clock();
                xSemaphoreGive(display_mutex);
            }
        }
        
        // REQUISITO: Temporização determinística com vTaskDelayUntil
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* ==== Funções de Calibração ==== */

static void calibrate_mpu(void) {
    printf("[CALIB] Iniciando calibração do MPU6050...\n");
    printf("[CALIB] Mantenha a placa parada e nivelada!\n");
    
    st7789_fill_screen_dma(COLOR_BLACK);
    st7789_draw_text_5x7(40, 100, "CALIBRANDO...", COLOR_YELLOW, 2, 0, 0);
    st7789_draw_text_5x7(30, 130, "Mantenha parado", COLOR_WHITE, 1, 0, 0);
    
    delay_ms(1000); // Espera estabilizar
    
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    const int samples = 50;
    mpu6050_raw_t raw;
    
    for (int i = 0; i < samples; i++) {
        if (mpu6050_read_all(&raw) == 0) {
            sum_x += raw.ax;
            sum_y += raw.ay;
        }
        delay_ms(20);
    }
    
    accel_offset_x = (int16_t)(sum_x / samples);
    accel_offset_y = (int16_t)(sum_y / samples);
    
    printf("[CALIB] Offsets definidos: X=%d, Y=%d\n", accel_offset_x, accel_offset_y);
    
    st7789_fill_screen_dma(COLOR_GREEN);
    st7789_draw_text_5x7(60, 110, "OK!", COLOR_BLACK, 3, 0, 0);
    delay_ms(500);
}

/* ==== Main ==== */

int main(void) {
    SystemCoreClockUpdate();
    delay_init();
    
    button_init();
    
    serial_stdio_init(115200);
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║  SIMULADOR DIGITAL DE LABIRINTO       ║\n");
    printf("║  Equipe: Alfons, Mateus, Guilherme    ║\n");
    printf("║  ELF74 - Sistemas Embarcados          ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Inicializar display
    st7789_init();
    st7789_fill_screen_dma(COLOR_BLUE); // Tela AZUL para teste de vida
    delay_ms(100);
    st7789_set_speed_div(2);
    printf("[OK] Display ST7789 inicializado\n");
    
    // Inicializar I2C e MPU6050
    i2c1_init_100k(50000000u);

    // --- DIAGNÓSTICO I2C ---
    printf("Procurando MPU6050...\n");
    uint8_t who = 0;
    int found_68 = i2c1_read_reg(0x68, 0x75, &who);
    if (found_68 == 0) printf(" > Encontrado em 0x68 (WHO_AM_I=0x%02X)\n", who);
    else               printf(" > Falha em 0x68\n");

    int found_69 = i2c1_read_reg(0x69, 0x75, &who);
    if (found_69 == 0) printf(" > Encontrado em 0x69 (WHO_AM_I=0x%02X)\n", who);
    else               printf(" > Falha em 0x69\n");
    // -----------------------

    if (mpu6050_init() < 0) {
        printf("[ERRO] MPU6050 não detectado!\n");
        st7789_fill_screen_dma(COLOR_RED);
        st7789_draw_text_5x7(20, 100, "MPU6050 ERROR", COLOR_WHITE, 2, 0, 0);
        for (;;) {
            delay_ms(500);
        }
    }
    printf("[OK] MPU6050 inicializado\n");
    
    // Calibrar MPU6050
    calibrate_mpu();
    
    // Mensagem inicial
    st7789_fill_screen_dma(COLOR_BLACK);
    st7789_draw_text_5x7(20, 90, "LABIRINTO DIGITAL", COLOR_GREEN, 2, 0, 0);
    st7789_draw_text_5x7(40, 120, "Inicializando...", COLOR_WHITE, 1, 0, 0);
    delay_ms(1500);
    
    /* REQUISITO: Objetos de Sincronização */
    printf("\n[INIT] Criando objetos de sincronização...\n");
    
    // REQUISITO: Queue para comunicação IMU -> GameLogic
    imu_queue = xQueueCreate(10, sizeof(mpu6050_raw_t));
    if (imu_queue == NULL) {
        printf("[ERRO] Falha ao criar IMU Queue\n");
        for (;;) {}
    }
    printf("[OK] Queue IMU criada (10 elementos)\n");
    
    // REQUISITO: Mutex para acesso ao display
    display_mutex = xSemaphoreCreateMutex();
    if (display_mutex == NULL) {
        printf("[ERRO] Falha ao criar Display Mutex\n");
        for (;;) {}
    }
    printf("[OK] Mutex Display criado\n");
    
    // REQUISITO: Semáforo binário para sincronização de estados
    game_state_sem = xSemaphoreCreateBinary();
    if (game_state_sem == NULL) {
        printf("[ERRO] Falha ao criar Game State Semaphore\n");
        for (;;) {}
    }
    printf("[OK] Semáforo Game State criado\n");
    
    // REQUISITO: Timer para relógio hh:mm:ss (1 segundo)
    clock_timer = xTimerCreate("ClockTimer",
                               pdMS_TO_TICKS(1000),  // 1 segundo
                               pdTRUE,                // Auto-reload
                               (void *)0,
                               vClockTimerCallback);
    if (clock_timer == NULL) {
        printf("[ERRO] Falha ao criar Clock Timer\n");
        for (;;) {}
    }
    printf("[OK] Timer Relógio criado (1Hz)\n");
    
    // Iniciar o timer do relógio
    if (xTimerStart(clock_timer, 0) != pdPASS) {
        printf("[ERRO] Falha ao iniciar Clock Timer\n");
        for (;;) {}
    }
    printf("[OK] Timer Relógio iniciado\n");
    
    /* REQUISITO: Criar mínimo 3 tarefas (criamos 5!) */
    printf("\n[INIT] Criando tarefas FreeRTOS...\n");
    
    BaseType_t ret;
    
    // Task 1: Leitura IMU (Prioridade 4 - Mais alta)
    ret = xTaskCreate(IMU_Task, "IMU", 256, NULL, 4, NULL);
    if (ret != pdPASS) {
        printf("[ERRO] Falha ao criar IMU_Task\n");
        for (;;) {}
    }
    printf("[OK] Task IMU criada (Pri:4, 50Hz)\n");
    
    // Task 2: Lógica do Jogo com Máquina de Estados (Prioridade 3)
    ret = xTaskCreate(GameLogic_Task, "LOGIC", 512, NULL, 3, NULL);
    if (ret != pdPASS) {
        printf("[ERRO] Falha ao criar GameLogic_Task\n");
        for (;;) {}
    }
    printf("[OK] Task GameLogic criada (Pri:3, 50Hz, FSM)\n");
    
    // Task 3: Renderização Display (Prioridade 2)
    ret = xTaskCreate(Display_Task, "DISP", 512, NULL, 2, NULL);
    if (ret != pdPASS) {
        printf("[ERRO] Falha ao criar Display_Task\n");
        for (;;) {}
    }
    printf("[OK] Task Display criada (Pri:2, 20Hz)\n");
    
    // Task 4: Leitura Botão (Prioridade 2)
    ret = xTaskCreate(Button_Task, "BTN", 128, NULL, 2, NULL);
    if (ret != pdPASS) {
        printf("[ERRO] Falha ao criar Button_Task\n");
        for (;;) {}
    }
    printf("[OK] Task Button criada (Pri:2, 50Hz)\n");
    
    // Task 5: Atualização Relógio Display (Prioridade 1 - Mais baixa)
    ret = xTaskCreate(ClockDisplay_Task, "CLK", 128, NULL, 1, NULL);
    if (ret != pdPASS) {
        printf("[ERRO] Falha ao criar ClockDisplay_Task\n");
        for (;;) {}
    }
    printf("[OK] Task ClockDisplay criada (Pri:1, 1Hz)\n");
    
    
    printf("\n[START] Iniciando scheduler FreeRTOS...\n");
    vTaskStartScheduler();
    
    // Nunca deve chegar aqui
    printf("[ERRO] Scheduler retornou!\n");
    for (;;) {}
}
