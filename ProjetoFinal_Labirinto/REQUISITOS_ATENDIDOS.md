# ✅ Requisitos do Projeto - Checklist Completo

## 📋 Requisitos Obrigatórios

### ✅ 1. Mínimo 3 threads/tarefas

**Status**: ✅ **ATENDIDO - 5 tarefas implementadas**

| # | Nome da Task | Arquivo | Linha | Frequência | Prioridade | Descrição |
|---|-------------|---------|-------|------------|------------|-----------|
| 1 | `IMU_Task` | main.c | ~395 | 50Hz | 4 (Mais alta) | Lê dados do MPU6050 |
| 2 | `GameLogic_Task` | main.c | ~413 | 50Hz | 3 | **Máquina de estados** do jogo |
| 3 | `Display_Task` | main.c | ~505 | 20Hz | 2 | Renderiza gráficos no LCD |
| 4 | `Button_Task` | main.c | ~543 | 50Hz | 2 | Lê entrada do botão |
| 5 | `ClockDisplay_Task` | main.c | ~563 | 1Hz | 1 (Mais baixa) | Atualiza relógio no display |

**Evidência no código**:
```c
// main.c, linhas ~655-690
xTaskCreate(IMU_Task,          "IMU",    256, NULL, 4, NULL);
xTaskCreate(GameLogic_Task,    "LOGIC",  512, NULL, 3, NULL);
xTaskCreate(Display_Task,      "DISP",   512, NULL, 2, NULL);
xTaskCreate(Button_Task,       "BTN",    128, NULL, 2, NULL);
xTaskCreate(ClockDisplay_Task, "CLK",    128, NULL, 1, NULL);
```

---

### ✅ 2. Pelo menos uma tarefa com Máquina de Estados

**Status**: ✅ **ATENDIDO - `GameLogic_Task`**

**Localização**: `src/main.c`, linhas ~413-500

**Estados Implementados**:

```c
typedef enum {
    GAME_INIT,       // Inicialização
    GAME_READY,      // Pronto para começar
    GAME_PLAYING,    // Jogando ativamente
    GAME_PAUSED,     // Pausado (reservado)
    GAME_LOST_LIFE,  // Perdeu uma vida
    GAME_WON,        // Venceu o jogo
    GAME_OVER        // Game Over
} GameState;
```

**Diagrama de Estados**:
```
    INIT
      ↓
    READY
      ↓
   PLAYING ←──────┐
      ↓           │
   (evento?)      │
      ↓           │
   ┌──┴──┐        │
   ↓     ↓        │
LOST_LIFE  →──────┘
   ↓
  WON/OVER
   ↓
 (botão)
   ↓
  INIT
```

**Evidência no código**:
```c
// main.c, linhas ~425-500
switch (game_state) {
    case GAME_INIT:
        printf("[STATE] INIT -> READY\n");
        // ... inicialização
        game_state = GAME_READY;
        break;
        
    case GAME_READY:
        printf("[STATE] READY -> PLAYING\n");
        game_state = GAME_PLAYING;
        xSemaphoreGive(game_state_sem);  // Sincronização
        break;
        
    case GAME_PLAYING:
        // ... lógica do jogo
        if (ball_check_hole()) {
            game_state = GAME_LOST_LIFE;  // Transição
        }
        break;
    // ... outros estados
}
```

---

### ✅ 3. Uso de objetos de sincronização (Fila, mutex, semáforos)

**Status**: ✅ **ATENDIDO - 4 objetos implementados**

#### 3.1 Queue (Fila)

**Nome**: `imu_queue`  
**Tipo**: `QueueHandle_t`  
**Tamanho**: 10 elementos de `mpu6050_raw_t`  
**Localização criação**: `src/main.c`, linha ~620  
**Uso**:
- **Produtor**: `IMU_Task` (linha ~407) - `xQueueSend()`
- **Consumidor**: `GameLogic_Task` (linha ~454) - `xQueueReceive()`

**Evidência**:
```c
// Criação (main.c ~620)
imu_queue = xQueueCreate(10, sizeof(mpu6050_raw_t));

// Envio (IMU_Task ~407)
xQueueSend(imu_queue, &imu_data, 0);

// Recebimento (GameLogic_Task ~454)
if (xQueueReceive(imu_queue, &imu_data, 0) == pdPASS) {
    ball_update_physics(dt, (float)imu_data.ay, (float)imu_data.ax);
}
```

#### 3.2 Mutex

**Nome**: `display_mutex`  
**Tipo**: `SemaphoreHandle_t`  
**Localização criação**: `src/main.c`, linha ~629  
**Uso**:
- `Display_Task` (linhas ~515, 537)
- `ClockDisplay_Task` (linhas ~573, 575)

**Propósito**: Proteger acesso concorrente ao display ST7789

**Evidência**:
```c
// Criação (main.c ~629)
display_mutex = xSemaphoreCreateMutex();

// Uso (Display_Task ~515)
if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    render_maze();
    render_ball();
    xSemaphoreGive(display_mutex);
}
```

#### 3.3 Semáforo Binário

**Nome**: `game_state_sem`  
**Tipo**: `SemaphoreHandle_t`  
**Localização criação**: `src/main.c`, linha ~638  
**Uso**: `GameLogic_Task` (linha ~441)

**Propósito**: Sinalizar mudanças de estado do jogo

**Evidência**:
```c
// Criação (main.c ~638)
game_state_sem = xSemaphoreCreateBinary();

// Uso (GameLogic_Task ~441)
xSemaphoreGive(game_state_sem);  // Sinaliza transição de estado
```

#### 3.4 Software Timer

**Nome**: `clock_timer`  
**Tipo**: `TimerHandle_t`  
**Período**: 1000ms (1 Hz)  
**Localização criação**: `src/main.c`, linha ~647  
**Callback**: `vClockTimerCallback` (linha ~256)

**Propósito**: Atualizar relógio hh:mm:ss a cada segundo

**Evidência**:
```c
// Criação (main.c ~647)
clock_timer = xTimerCreate("ClockTimer",
                           pdMS_TO_TICKS(1000),  // 1 segundo
                           pdTRUE,                // Auto-reload
                           (void *)0,
                           vClockTimerCallback);

// Callback (main.c ~256)
static void vClockTimerCallback(TimerHandle_t xTimer) {
    system_clock.seconds++;
    if (system_clock.seconds >= 60) {
        system_clock.seconds = 0;
        system_clock.minutes++;
        // ...
    }
    clock_updated = 1;
}
```

---

### ✅ 4. Uso de temporização determinística

**Status**: ✅ **ATENDIDO - vTaskDelayUntil em 5 tarefas**

**Implementação**: Todas as tarefas usam `vTaskDelayUntil()` ao invés de `vTaskDelay()`

| Task | Localização | Período | Evidência |
|------|------------|---------|-----------|
| IMU_Task | main.c ~410 | 20ms (50Hz) | `vTaskDelayUntil(&xLastWakeTime, xPeriod)` |
| GameLogic_Task | main.c ~499 | 20ms (50Hz) | `vTaskDelayUntil(&xLastWakeTime, xPeriod)` |
| Display_Task | main.c ~539 | 50ms (20Hz) | `vTaskDelayUntil(&xLastWakeTime, xPeriod)` |
| Button_Task | main.c ~559 | 20ms (50Hz) | `vTaskDelayUntil(&xLastWakeTime, xPeriod)` |
| ClockDisplay_Task | main.c ~581 | 1000ms (1Hz) | `vTaskDelayUntil(&xLastWakeTime, xPeriod)` |

**Padrão usado em todas as tasks**:
```c
static void Task_Name(void *arg) {
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(XX);  // Período fixo
    
    xLastWakeTime = xTaskGetTickCount();  // Inicializar
    
    for (;;) {
        // ... trabalho da task ...
        
        // REQUISITO: Temporização determinística
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}
```

**Vantagens sobre `vTaskDelay()`**:
- ✅ Execução periódica precisa
- ✅ Não acumula drift temporal
- ✅ Garante período constante independente do tempo de execução
- ✅ Ideal para controle em tempo real

**Uso adicional de TickCount**:
```c
// GameLogic_Task (main.c ~450)
game_time_ms = (xTaskGetTickCount() - start_ticks) * portTICK_PERIOD_MS;
```

---

### ✅ 5. Relógio hh:mm:ss no display (atualização a cada segundo)

**Status**: ✅ **ATENDIDO**

**Implementação**: 

#### Estrutura de Dados:
```c
// main.c ~60
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} ClockTime;

static ClockTime system_clock = {0, 0, 0};
```

#### Atualização (Timer Callback):
```c
// main.c ~256
static void vClockTimerCallback(TimerHandle_t xTimer) {
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
    clock_updated = 1;  // Flag para task de display
}
```

#### Renderização:
```c
// main.c ~276
static void render_clock(void) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", 
             system_clock.hours, 
             system_clock.minutes, 
             system_clock.seconds);
    st7789_draw_text_5x7(5, 12, buf, COLOR_YELLOW, 1, 0, 0);
}
```

#### Task de Atualização Display:
```c
// main.c ~563
static void ClockDisplay_Task(void *arg) {
    for (;;) {
        if (clock_updated) {
            clock_updated = 0;
            if (xSemaphoreTake(display_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                render_clock();  // Atualiza display
                xSemaphoreGive(display_mutex);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));  // 1 segundo
    }
}
```

**Localização visual no display**:
- Posição: Canto superior esquerdo (x=5, y=12)
- Cor: Amarelo (`COLOR_YELLOW`)
- Formato: `HH:MM:SS` (ex: `00:05:42`)
- Sempre visível durante o jogo

---

## 📊 Resumo de Atendimento aos Requisitos

| # | Requisito | Status | Implementação |
|---|-----------|--------|---------------|
| 1 | Mínimo 3 threads/tarefas | ✅ **5 tarefas** | IMU, GameLogic, Display, Button, ClockDisplay |
| 2 | Máquina de estados | ✅ **7 estados** | GameLogic_Task com FSM completa |
| 3 | Queue (Fila) | ✅ **1 queue** | `imu_queue` para dados MPU6050 |
| 4 | Mutex | ✅ **1 mutex** | `display_mutex` para acesso display |
| 5 | Semáforo | ✅ **1 semáforo** | `game_state_sem` para sincronização |
| 6 | Timer | ✅ **1 timer** | `clock_timer` para relógio 1Hz |
| 7 | Temporização determinística | ✅ **5 tarefas** | Todas usam `vTaskDelayUntil()` |
| 8 | Uso de TickCount | ✅ **Sim** | Timer do jogo e timestamps |
| 9 | Relógio hh:mm:ss | ✅ **Sim** | Atualização a cada 1s, sempre visível |

---

## 🎯 Extras Implementados (além dos requisitos)

1. **Sistema de física realista** - Aceleração, velocidade, atrito
2. **Detecção de colisões** - 8 pontos ao redor da bolinha
3. **Sistema de vidas** - 3 vidas com reset de posição
4. **Melhor tempo** - Salva e exibe o melhor tempo
5. **HUD completo** - Vidas, tempo do jogo, relógio
6. **Debug via UART** - Logs de estados e eventos
7. **Labirinto 16x16** - Com paredes, buracos e objetivo
8. **Cores variadas** - Interface visual atraente

---

## 📝 Como Verificar Cada Requisito

### Verificação em Tempo de Compilação:
```bash
cd ProjetoFinal_Labirinto
pio run
# Deve compilar sem erros
```

### Verificação em Tempo de Execução:

1. **Tarefas**: Monitor serial mostra criação de 5 tarefas
```
[OK] Task IMU criada (Pri:4, 50Hz)
[OK] Task GameLogic criada (Pri:3, 50Hz, FSM)
[OK] Task Display criada (Pri:2, 20Hz)
[OK] Task Button criada (Pri:2, 50Hz)
[OK] Task ClockDisplay criada (Pri:1, 1Hz)
```

2. **Máquina de Estados**: Logs mostram transições
```
[STATE] INIT -> READY
[STATE] READY -> PLAYING
[STATE] PLAYING -> LOST_LIFE
```

3. **Sincronização**: Código usa explicitamente:
   - `xQueueSend()` / `xQueueReceive()`
   - `xSemaphoreTake()` / `xSemaphoreGive()`
   - `xTimerCreate()` / `xTimerStart()`

4. **Temporização**: Todas tasks usam `vTaskDelayUntil()`

5. **Relógio**: Display mostra `HH:MM:SS` atualizando a cada segundo

---

## 📄 Arquivo de Evidência

**Arquivo principal**: `src/main.c`  
**Linhas de código**: ~700 linhas

**Principais seções**:
- Linhas 1-50: Includes e comentários dos requisitos
- Linhas 60-90: Estruturas e enums (incluindo FSM)
- Linhas 110-130: Variáveis globais e objetos de sincronização
- Linhas 256-275: Timer callback para relógio
- Linhas 395-585: 5 Tasks FreeRTOS
- Linhas 590-740: Função main() com criação de objetos

---

## ✅ Conclusão

**TODOS OS REQUISITOS FORAM ATENDIDOS E IMPLEMENTADOS CORRETAMENTE!**

O projeto demonstra uso avançado de:
- ✅ FreeRTOS (5 tarefas)
- ✅ Máquina de estados (7 estados)
- ✅ Sincronização (Queue, Mutex, Semaphore, Timer)
- ✅ Temporização determinística (vTaskDelayUntil)
- ✅ Relógio em tempo real (hh:mm:ss)

---

**Data**: 18/11/2025  
**Projeto**: Simulador Digital de Labirinto  
**Equipe**: Alfons, Mateus, Guilherme  
**Disciplina**: ELF74 - Sistemas Embarcados  
**Professor**: Eduardo Nunes Dos Santos
