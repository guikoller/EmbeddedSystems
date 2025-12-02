# Explicação do Código e FreeRTOS

Este documento detalha a implementação do firmware do **Simulador Digital de Labirinto**, com foco especial na utilização do sistema operacional de tempo real **FreeRTOS**.

## 1. Estrutura Geral do Firmware

O código é estruturado em torno de **Tarefas (Tasks)** independentes que executam concorrentemente, gerenciadas pelo escalonador (scheduler) do FreeRTOS. A comunicação e sincronização entre essas tarefas são feitas através de **Filas (Queues)**, **Semáforos (Semaphores)** e **Mutexes**.

### Arquitetura de Tarefas

O sistema possui 5 tarefas principais, definidas em `src/main.c`:

| Tarefa | Prioridade | Frequência | Descrição |
| :--- | :---: | :---: | :--- |
| **IMU_Task** | 4 (Alta) | ~30Hz | Lê os dados brutos do acelerômetro (MPU6050) e envia para a fila. |
| **GameLogic_Task** | 3 | ~30Hz | Recebe dados da fila, calcula a física, colissões e gerencia a Máquina de Estados do jogo. |
| **Display_Task** | 2 | 10Hz | Renderiza o labirinto, a bolinha e o HUD no display LCD. |
| **Button_Task** | 2 | 10Hz | Lê o estado do botão para reiniciar o jogo. |
| **ClockDisplay_Task** | 1 (Baixa) | 1Hz | Atualiza apenas a área do relógio na tela a cada segundo. |

---

## 2. Conceitos e Funções do FreeRTOS Utilizados

Abaixo estão explicadas as principais funções do FreeRTOS utilizadas no projeto e o *porquê* de seu uso.

### 2.1. Criação e Gerenciamento de Tarefas

*   **`xTaskCreate(...)`**
    *   **Uso:** Cria uma nova tarefa e a adiciona à lista de prontos para execução.
    *   **No Código:** Usada na `main()` para criar as 5 tarefas.
    *   **Parâmetros Importantes:** Ponteiro da função da tarefa, nome (debug), tamanho da pilha (stack), parâmetros, prioridade e handle.

*   **`vTaskStartScheduler()`**
    *   **Uso:** Inicia o kernel do FreeRTOS. A partir deste ponto, o controle é passado para as tarefas e a execução sequencial da `main()` para.

*   **`vTaskDelayUntil(&xLastWakeTime, xPeriod)`**
    *   **Uso:** Garante uma **frequência de execução exata**. Diferente do `vTaskDelay` (que espera um tempo *após* a execução), esta função acorda a tarefa no tempo absoluto calculado.
    *   **No Código:** Usada em todas as tarefas (`IMU_Task`, `GameLogic_Task`, etc.) para garantir que o jogo rode a 30Hz e o display a 10Hz de forma estável (Jitter baixo).

*   **`xTaskGetTickCount()`**
    *   **Uso:** Retorna o tempo atual do sistema em "ticks". Necessário para inicializar o `vTaskDelayUntil`.

### 2.2. Comunicação entre Tarefas (Queues)

*   **`xQueueCreate(len, size)`**
    *   **Uso:** Cria uma fila de mensagens.
    *   **No Código:** `imu_queue = xQueueCreate(10, sizeof(mpu6050_raw_t));`
    *   **Objetivo:** Criar um buffer seguro para passar dados do sensor para a lógica do jogo.

*   **`xQueueSend(queue, &data, timeout)`**
    *   **Uso:** Envia um item para a fila.
    *   **No Código:** A `IMU_Task` envia os dados lidos do acelerômetro. Se a fila estiver cheia, o dado é descartado (timeout 0).

*   **`xQueueReceive(queue, &data, timeout)`**
    *   **Uso:** Recebe um item da fila.
    *   **No Código:** A `GameLogic_Task` espera por dados. Isso sincroniza a lógica com a leitura do sensor.

### 2.3. Proteção de Recursos Compartilhados (Mutex)

*   **`xSemaphoreCreateMutex()`**
    *   **Uso:** Cria um Mutex (Mutual Exclusion).
    *   **No Código:** `display_mutex`.
    *   **Objetivo:** O display SPI é um recurso único que não pode ser acessado por duas tarefas ao mesmo tempo.

*   **`xSemaphoreTake(mutex, timeout)`**
    *   **Uso:** Tenta pegar o "bastão" (lock) do Mutex. Se outra tarefa já pegou, espera até liberar ou o tempo acabar.
    *   **No Código:** Antes de desenhar qualquer coisa (`st7789_...`), as tarefas `Display_Task` e `ClockDisplay_Task` chamam essa função.

*   **`xSemaphoreGive(mutex)`**
    *   **Uso:** Devolve o "bastão", liberando o recurso para outras tarefas.
    *   **No Código:** Chamado imediatamente após terminar de desenhar.

### 2.4. Temporizadores de Software (Software Timers)

*   **`xTimerCreate(...)`**
    *   **Uso:** Cria um timer que executa uma função de callback periodicamente, gerenciado pelo daemon do FreeRTOS (não é uma tarefa de hardware).
    *   **No Código:** `clock_timer` configurado para 1000ms (1 segundo).

*   **`vClockTimerCallback(...)`**
    *   **Uso:** Função chamada quando o timer estoura.
    *   **No Código:** Incrementa a estrutura `system_clock` (segundos, minutos, horas). Isso mantém o relógio preciso independentemente da carga da CPU nas outras tarefas.

## 3. Fluxo de Dados Simplificado

1.  **Hardware (MPU6050)** -> I2C -> **`IMU_Task`**
2.  **`IMU_Task`** -> `xQueueSend` -> **`imu_queue`**
3.  **`imu_queue`** -> `xQueueReceive` -> **`GameLogic_Task`**
4.  **`GameLogic_Task`** -> Atualiza variáveis globais (`ball`, `game_state`)
5.  **`Display_Task`** -> Lê variáveis globais -> `xSemaphoreTake` -> **Display SPI** -> `xSemaphoreGive`

## 4. Por que usar FreeRTOS aqui?

1.  **Determinismo:** A física do jogo precisa rodar em intervalos constantes (dt fixo) para que a integração da velocidade e posição seja correta. O `vTaskDelayUntil` garante isso.
2.  **Responsividade:** A leitura do sensor (IMU) tem alta prioridade. Se o desenho na tela demorar, a leitura do sensor não é atrasada, pois o RTOS preempte a tarefa de display.
3.  **Organização:** Separar a lógica do jogo, a leitura de sensores e a atualização de tela em módulos distintos facilita a manutenção e a evolução do código (como adicionar o seletor de mapas).
