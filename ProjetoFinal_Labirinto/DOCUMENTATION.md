# Documentação do Projeto - Simulador Digital de Labirinto

## Visão Geral

Este projeto implementa um jogo de labirinto digital inspirado nos brinquedos tradicionais de labirinto físico. O jogador inclina a placa Black Pill para mover uma bolinha virtual através do labirinto até alcançar o objetivo, evitando cair nos buracos.

## Hardware Utilizado

- **Microcontrolador**: STM32F411CEU6 (Black Pill)
- **Display**: ST7789 240x240 SPI LCD
- **Sensor**: MPU6050 (Acelerômetro + Giroscópio)
- **Interface**: Botão para reset (PA0)
- **Debug**: UART (115200 baud)

## Arquitetura do Software

### FreeRTOS Tasks

#### 1. IMU_Task (Prioridade 3 - Alta)
- **Frequência**: 50 Hz
- **Função**: Lê dados do MPU6050 (acelerômetro e giroscópio)
- **Comunicação**: Envia dados via queue para GameLogic_Task

#### 2. GameLogic_Task (Prioridade 2 - Média)
- **Frequência**: 50 Hz
- **Função**: 
  - Processa física da bolinha
  - Detecta colisões
  - Verifica condições de vitória/derrota
  - Gerencia estados do jogo
- **Entrada**: Dados do IMU via queue
- **Saída**: Atualiza estado do jogo

#### 3. Display_Task (Prioridade 1 - Baixa)
- **Frequência**: ~20 FPS (50ms)
- **Função**: Renderiza gráficos no LCD
- **Sincronização**: Usa mutex para acesso ao display

#### 4. Button_Task (Prioridade 1 - Baixa)
- **Frequência**: 50 Hz
- **Função**: Lê botão e gerencia debouncing
- **Função**: Sinaliza reset do jogo

### Máquina de Estados do Jogo

```
GAME_INIT → GAME_PLAYING → {GAME_WON, GAME_OVER}
     ↑                              ↓
     └──────────← (botão) ←─────────┘
```

#### Estados:

1. **GAME_INIT**: Inicializa labirinto, bolinha, vidas
2. **GAME_PLAYING**: Loop principal do jogo
3. **GAME_WON**: Jogador alcançou o objetivo
4. **GAME_OVER**: Perdeu todas as vidas
5. **GAME_LOST_LIFE**: Transição quando cai em buraco

## Física da Bolinha

### Modelo Físico

A bolinha segue um modelo de física 2D simplificado:

```c
// Aceleração baseada na inclinação
ax = tilt_x * GRAVITY * TILT_SCALE
ay = tilt_y * GRAVITY * TILT_SCALE

// Integração de velocidade
vx += ax * dt
vy += ay * dt

// Atrito
vx *= FRICTION  // 0.92
vy *= FRICTION

// Integração de posição
x += vx * dt
y += vy * dt
```

### Parâmetros Ajustáveis

- `GRAVITY`: 200.0 (aceleração base)
- `FRICTION`: 0.92 (coeficiente de atrito)
- `TILT_SCALE`: 0.0015 (sensibilidade do acelerômetro)
- `MAX_VELOCITY`: 80.0 (velocidade máxima em pixels/s)

### Detecção de Colisão

O sistema verifica 8 pontos ao redor da bolinha (a cada 45°) para detectar colisões com paredes:

```
     0°
  315° | 45°
270° -O- 90°
  225° | 135°
    180°
```

## Estrutura do Labirinto

### Dimensões
- **Tamanho**: 16x16 células
- **Célula**: 14x14 pixels
- **Offset**: (8, 20) pixels
- **Total**: 224x224 pixels de área de jogo

### Tipos de Células

| Valor | Tipo    | Cor   | Descrição                |
|-------|---------|-------|--------------------------|
| 0     | EMPTY   | Branco| Caminho livre            |
| 1     | WALL    | Cinza | Parede sólida            |
| 2     | HOLE    | Preto | Buraco (perde vida)      |
| 3     | GOAL    | Verde | Objetivo (vitória)       |

### Mapa Atual

O labirinto atual tem:
- **Início**: (1, 1)
- **Objetivo**: (14, 14)
- **Buracos**: 3 localizações estratégicas
- **Dificuldade**: Média

## Interface Gráfica

### HUD (Head-Up Display)

- **Vidas**: Canto superior esquerdo
- **Tempo**: Canto superior direito (formato: MM.SSS)

### Telas

1. **Jogo**: Labirinto + Bolinha + HUD
2. **Vitória**: "YOU WIN!" + tempo + melhor tempo
3. **Game Over**: "GAME OVER" + tempo final

## Configuração e Compilação

### Pré-requisitos

1. **FreeRTOS-Kernel**: Seguir instruções em `lib/FREERTOS_SETUP.md`
2. **PlatformIO**: Instalado no VS Code
3. **ST-Link**: Para upload e debug

### Compilar

```bash
cd ProjetoFinal_Labirinto
pio run
```

### Upload

```bash
pio run -t upload
```

### Monitor Serial

```bash
pio device monitor
```

## Melhorias Futuras

### Funcionalidades Planejadas

1. **Múltiplos Níveis**: Sistema de fases progressivas
2. **Highscore**: Salvar melhores tempos na EEPROM
3. **Efeitos Sonoros**: Buzzer para feedback
4. **Power-ups**: Itens especiais no labirinto
5. **Menu**: Seleção de dificuldade e fases
6. **Animações**: Efeitos visuais para eventos
7. **Calibração**: Opção de calibrar o IMU

### Otimizações

1. **Renderização**: Apenas redesenhar células modificadas
2. **Física**: Usar ponto fixo ao invés de float
3. **Memória**: Compressão do mapa do labirinto
4. **Energia**: Modo sleep quando inativo

## Debug

### Mensagens Serial

O código envia informações via UART para debug:

```
=== SIMULADOR DE LABIRINTO ===
Equipe: Alfons, Mateus, Guilherme

Display OK
MPU6050 OK
Starting scheduler...
Game started!
Fell in hole! Lives: 2
You Won! Time: 15432 ms
```

### Problemas Comuns

1. **MPU6050 não inicializa**
   - Verificar conexões I2C (SDA, SCL)
   - Verificar endereço I2C (0x68 ou 0x69)

2. **Display não funciona**
   - Verificar conexões SPI
   - Verificar alimentação (3.3V)

3. **Física muito rápida/lenta**
   - Ajustar `TILT_SCALE`
   - Ajustar `FRICTION`

4. **FreeRTOS não compila**
   - Verificar `lib/FREERTOS_SETUP.md`
   - Conferir `platformio.ini`

## Referências

- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [ST7789 Datasheet](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)

## Licença

Projeto acadêmico - UTFPR Campus Curitiba
ELF74 - Sistemas Embarcados
Prof. Eduardo Nunes Dos Santos
