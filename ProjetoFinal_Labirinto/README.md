# Projeto Final - Simulador Digital de Labirinto 🎮

<div align="center">

**ELF74 - Sistemas Embarcados**  
Universidade Tecnológica Federal do Paraná - Campus Curitiba  
Prof. Eduardo Nunes Dos Santos

</div>

---

## 👥 Equipe

- **Alfons Carlos Cesar Heiermann de Andrade**
- **Mateus Filipe de Ornelas Rampim**
- **Guilherme Corrêa Koller**

---

## 📝 Descrição

Implementação de um **jogo de labirinto digital** inspirado nos brinquedos tradicionais de labirinto físico. O jogador controla uma esfera virtual que se move pelo labirinto inclinando a placa microcontroladora, utilizando o sensor inercial (IMU) para detectar a inclinação em tempo real.

### 🎯 Objetivo do Jogo

Conduzir a esfera desde o ponto inicial até o objetivo final do labirinto, evitando cair nos buracos e desviando das paredes, no menor tempo possível.

---

## 🔧 Hardware Utilizado

| Componente | Modelo | Descrição |
|------------|--------|-----------|
| **Microcontrolador** | STM32F411CEU6 (Black Pill) | Processamento principal |
| **Display** | ST7789 240x240 LCD | Visualização do jogo |
| **Sensor Inercial** | MPU6050 | Acelerômetro + Giroscópio |
| **RTOS** | FreeRTOS | Sistema operacional em tempo real |
| **Interface** | Botão + UART | Controle e debug |

---

## 🎯 Requisitos Obrigatórios Implementados

### ✅ Todos os requisitos foram atendidos!

1. **Mínimo 3 threads/tarefas** → ✅ **5 tarefas criadas**
   - IMU_Task, GameLogic_Task, Display_Task, Button_Task, ClockDisplay_Task

2. **Máquina de estados** → ✅ **7 estados implementados**
   - GAME_INIT, GAME_READY, GAME_PLAYING, GAME_PAUSED, GAME_LOST_LIFE, GAME_WON, GAME_OVER

3. **Objetos de sincronização** → ✅ **4 objetos**
   - Queue (imu_queue)
   - Mutex (display_mutex)
   - Semáforo Binário (game_state_sem)
   - Software Timer (clock_timer)

4. **Temporização determinística** → ✅ **vTaskDelayUntil em todas as tasks**
   - Períodos fixos: 50Hz (IMU), 50Hz (Logic), 20Hz (Display), 50Hz (Button), 1Hz (Clock)

5. **Relógio hh:mm:ss** → ✅ **Sempre visível no display**
   - Atualização a cada 1 segundo via Software Timer
   - Formato: HH:MM:SS (amarelo, canto superior esquerdo)

📄 **Veja detalhes completos em**: [REQUISITOS_ATENDIDOS.md](REQUISITOS_ATENDIDOS.md)

---

## ✨ Características Principais

### 🎮 Gameplay
- ✅ Controle por inclinação (física realista)
- ✅ Sistema de vidas (3 vidas)
- ✅ Cronômetro e melhor tempo
- ✅ Múltiplos obstáculos (paredes e buracos)
- ✅ Reset via botão
- ✅ **Relógio em tempo real (hh:mm:ss)**

### 🔬 Técnicas Avançadas
- ✅ **5 Tarefas FreeRTOS** com prioridades diferentes
- ✅ **Máquina de Estados Finitos** (FSM) completa
- ✅ **Sincronização avançada** (Queue, Mutex, Semaphore, Timer)
- ✅ **Temporização determinística** (vTaskDelayUntil)
- ✅ Física 2D em tempo real
- ✅ Detecção de colisões
- ✅ Renderização gráfica 20 FPS
- ✅ Processamento paralelo otimizado

---

## 📖 Documentação

### 🚀 Para Começar
1. **[QUICKSTART.md](QUICKSTART.md)** - **👈 COMECE AQUI!** Setup rápido e instruções básicas
2. **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Resumo rápido do projeto

### 📚 Documentação Detalhada
3. **[DOCUMENTATION.md](DOCUMENTATION.md)** - Arquitetura técnica completa
4. **[MAZE_EXAMPLES.md](MAZE_EXAMPLES.md)** - Mapas alternativos de labirinto
5. **[lib/FREERTOS_SETUP.md](lib/FREERTOS_SETUP.md)** - Como instalar FreeRTOS

---

## 🎮 Como Jogar

### Controles
- **Inclinação**: Move a esfera pelo labirinto (via MPU6050)
  - Frente/Trás: Eixo Y
  - Esquerda/Direita: Eixo X
- **Botão (PA0)**: Reiniciar jogo

### Regras
- 🟢 **Objetivo**: Chegar na célula verde
- ⚫ **Buracos**: Perde uma vida se cair
- ⚪ **Paredes**: Bloqueiam o caminho
- ❤️ **Vidas**: 3 chances para completar
- ⏱️ **Tempo**: Tente o melhor tempo!

---

## 🏗️ Estrutura do Projeto

```
ProjetoFinal_Labirinto/
├── 📄 platformio.ini         # Configuração PlatformIO
├── 📄 README.md              # Este arquivo
├── 📄 QUICKSTART.md          # Início rápido
├── 📄 DOCUMENTATION.md       # Docs técnicas
├── 📄 MAZE_EXAMPLES.md       # Mapas alternativos
├── 📄 PROJECT_SUMMARY.md     # Resumo executivo
│
├── 📂 include/               # Headers (.h)
│   ├── board.h               # Definições da placa
│   ├── delay_rtos.h          # Delays FreeRTOS
│   ├── font5x7.h             # Fonte para LCD
│   ├── FreeRTOSConfig.h      # Config FreeRTOS
│   ├── main.h                # Header principal
│   ├── mpu6050.h             # Driver IMU
│   ├── serial_stdio.h        # UART/Debug
│   └── st7789.h              # Driver LCD
│
├── 📂 src/                   # Código fonte (.c)
│   ├── ⭐ main.c            # ⭐ LÓGICA PRINCIPAL DO JOGO
│   ├── delay_rtos.c          # Implementação delays
│   ├── heap_4.c              # Alocação memória FreeRTOS
│   ├── mpu6050.c             # Implementação IMU
│   ├── serial_stdio.c        # Implementação UART
│   └── st7789.c              # Implementação LCD
│
└── 📂 lib/                   # Bibliotecas
    ├── FREERTOS_SETUP.md     # Setup FreeRTOS
    └── FreeRTOS-Kernel/      # Biblioteca FreeRTOS
```

---

## 🚀 Build e Upload

### Pré-requisitos
1. PlatformIO instalado no VS Code
2. FreeRTOS-Kernel configurado (ver `lib/FREERTOS_SETUP.md`)
3. ST-Link conectado à Black Pill

### Comandos

```bash
# Compilar
pio run

# Upload para a placa
pio run -t upload

# Monitor serial (debug)
pio device monitor

# Tudo de uma vez
pio run -t upload && pio device monitor
```

---

## 🔌 Conexões de Hardware

### Display ST7789 (SPI1)
- VCC → 3.3V
- GND → GND
- SCL → PA5 (SPI1_SCK)
- SDA → PA7 (SPI1_MOSI)
- RES → PB0 (Reset)
- DC → PB1 (Data/Command)
- CS → PA4 (Chip Select)
- BLK → 3.3V (Backlight)

### MPU6050 (I2C1)
- VCC → 3.3V
- GND → GND
- SCL → PB8 (I2C1_SCL)
- SDA → PB9 (I2C1_SDA)
- AD0 → GND (endereço 0x68)

### Botão
- BTN → PA0
- Outro lado → GND

### Debug UART (USART2)
- TX → PA2
- RX → PA3

---

## 🎨 Screenshots Conceituais

### Tela de Jogo
```
┌─────────────────────────────┐
│ LIVES:3      TIME:00.500    │ ← HUD
├─────────────────────────────┤
│ ┌─────────────────────────┐ │
│ │ ███████████████████████ │ │
│ │ █ ○             █     █ │ │ ← Bolinha (vermelha)
│ │ █ █ █████████████ █ █ █ │ │
│ │ █   █       ●         █ │ │ ← Buraco (preto)
│ │ ███████ █████ ████████ █ │ │
│ │ █     █     █       █ █ │ │
│ │ █ ███████ █ ███████ ███ │ │
│ │ █               █     ▓ │ │ ← Objetivo (verde)
│ │ ███████████████████████ │ │
│ └─────────────────────────┘ │
└─────────────────────────────┘
```

---

## 🐛 Troubleshooting

| Problema | Solução |
|----------|---------|
| MPU6050 não detecta | Verificar conexões I2C (PB8, PB9) |
| Display não acende | Verificar SPI e backlight (3.3V) |
| FreeRTOS não compila | Ver `lib/FREERTOS_SETUP.md` |
| Bolinha se move sozinha | Colocar placa em superfície plana ao ligar |
| Muito rápido/lento | Ajustar `TILT_SCALE` em `main.c` |

---

## 📈 Melhorias Futuras

- [ ] Múltiplos níveis progressivos
- [ ] Highscore persistente (EEPROM)
- [ ] Efeitos sonoros (buzzer)
- [ ] Power-ups coletáveis
- [ ] Menu de configuração
- [ ] Animações de transição
- [ ] Multiplayer via wireless

---

## 📚 Referências

- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [ST7789 Datasheet](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)

---

## 📜 Licença

Projeto acadêmico desenvolvido para a disciplina ELF74 - Sistemas Embarcados  
**UTFPR - Universidade Tecnológica Federal do Paraná**  
**Campus Curitiba - Departamento Acadêmico de Eletrônica**  
**2025**

---

<div align="center">

**🎮 Divirta-se jogando! 🎮**

Para começar: leia **[QUICKSTART.md](QUICKSTART.md)**

</div>
