# Resumo do Projeto - Simulador Digital de Labirinto

## 🎮 Visão Geral

**Jogo de labirinto digital** onde o jogador inclina a placa Black Pill para mover uma bolinha virtual através de um labirinto até alcançar o objetivo, evitando buracos.

**Equipe**: Alfons, Mateus, Guilherme  
**Curso**: ELF74 - Sistemas Embarcados - UTFPR  
**Prof.**: Eduardo Nunes Dos Santos

---

## 🔧 Hardware

| Componente | Modelo | Interface | Pinos |
|------------|--------|-----------|-------|
| MCU | STM32F411CEU6 (Black Pill) | - | - |
| Display | ST7789 240x240 | SPI1 | PA4,PA5,PA7,PB0,PB1 |
| IMU | MPU6050 | I2C1 | PB8,PB9 |
| Botão | Push Button | GPIO | PA0 |
| Debug | UART | USART2 | PA2,PA3 |

---

## 📁 Estrutura de Arquivos

```
ProjetoFinal_Labirinto/
├── 📄 README.md              - Informações gerais
├── 📄 QUICKSTART.md          - Início rápido (START HERE!)
├── 📄 DOCUMENTATION.md       - Documentação técnica completa
├── 📄 MAZE_EXAMPLES.md       - Mapas alternativos de labirinto
├── 📄 platformio.ini         - Configuração PlatformIO
├── 📂 include/               - Headers (.h)
├── 📂 src/                   - Código fonte (.c)
│   └── ⭐ main.c            - LÓGICA PRINCIPAL DO JOGO
└── 📂 lib/
    ├── FREERTOS_SETUP.md    - Instruções FreeRTOS
    └── FreeRTOS-Kernel/     - Biblioteca FreeRTOS
```

---

## 🚀 Quick Start

### 1. Setup FreeRTOS
```bash
cd lib
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd FreeRTOS-Kernel
git checkout V10.5.1
cd ../..
```

### 2. Build & Upload
```bash
pio run -t upload
```

### 3. Jogar
- Incline a placa para mover a bolinha
- Chegue até a célula verde (objetivo)
- Evite buracos (células pretas)
- Pressione botão para reiniciar

---

## 🎯 Características Principais

### Física Realista
- Aceleração baseada na inclinação (MPU6050)
- Atrito e velocidade máxima
- Colisão com paredes
- Gravidade simulada

### Sistema de Jogo
- ❤️ 3 vidas
- ⏱️ Cronômetro
- 🏆 Melhor tempo
- 🔄 Reset via botão

### Arquitetura FreeRTOS
- **4 Tasks paralelas**:
  - IMU_Task (50Hz) - Leitura sensor
  - GameLogic_Task (50Hz) - Física e lógica
  - Display_Task (20Hz) - Renderização
  - Button_Task (50Hz) - Entrada usuário

---

## 📊 Especificações Técnicas

### Labirinto
- Tamanho: 16x16 células
- Célula: 14x14 pixels
- Total: 224x224 pixels área de jogo
- Tipos: Vazio(0), Parede(1), Buraco(2), Objetivo(3)

### Física
- Gravidade: 200.0 pixels/s²
- Atrito: 0.92
- Vel. Máxima: 80.0 pixels/s
- Raio Bolinha: 4 pixels

### Performance
- Taxa de atualização física: 50 Hz
- Taxa de renderização: 20 FPS
- Latência IMU: 20ms
- Debounce botão: 20ms

---

## 🎨 Interface

### Tela Principal (Jogo)
```
┌─────────────────────────────┐
│ LIVES:3      TIME:00.500    │
├─────────────────────────────┤
│ ┌─────────────────────────┐ │
│ │ ███████████████████████ │ │
│ │ █   █             █   █ │ │
│ │ █ █ █████████████ █ █ █ │ │
│ │ █     ○       █       █ │ │  ← Bolinha (○)
│ │ ███████ █████ ████████ █ │ │
│ │ █     █     █     ●   █ │ │  ← Buraco (●)
│ │ █ ███████ █ ███████ ███ │ │
│ │ █               █     █ │ │
│ │ █████████████ █ █ █ █ █ │ │
│ │ █     ●     █ █ █ █ █ █ │ │
│ │ █ ███████ █ █ █ █ █ █ █ │ │
│ │ █     █   █   █   █ █ ▓ │ │  ← Objetivo (▓)
│ │ ███████████████████████ │ │
│ └─────────────────────────┘ │
└─────────────────────────────┘
```

### Tela de Vitória
```
┌─────────────────────────────┐
│                             │
│       YOU WIN! 🎉          │
│                             │
│    TIME: 15.432 s          │
│    BEST: 12.108 s          │
│                             │
│  Press button to restart    │
└─────────────────────────────┘
```

---

## 🔧 Ajustes Comuns

### Sensibilidade do Controle
Em `src/main.c`:

```c
// Mais sensível (bolinha move mais rápido)
#define TILT_SCALE 0.0025f  // padrão: 0.0015f

// Menos atrito (desliza mais)
#define FRICTION 0.88f      // padrão: 0.92f

// Mais rápida
#define MAX_VELOCITY 120.0f // padrão: 80.0f
```

### Mudar Labirinto
Veja `MAZE_EXAMPLES.md` para mapas prontos.

---

## 📚 Documentação

| Arquivo | Descrição |
|---------|-----------|
| `README.md` | Visão geral e objetivos |
| `QUICKSTART.md` | **👉 COMECE AQUI** - Setup rápido |
| `DOCUMENTATION.md` | Arquitetura técnica detalhada |
| `MAZE_EXAMPLES.md` | Mapas alternativos prontos |
| `PROJECT_SUMMARY.md` | Este arquivo - resumo rápido |
| `lib/FREERTOS_SETUP.md` | Como instalar FreeRTOS |

---

## 🐛 Troubleshooting

| Problema | Solução |
|----------|---------|
| MPU6050 não detecta | Verificar I2C: SDA(PB9), SCL(PB8) |
| Display não acende | Verificar SPI e backlight (3.3V) |
| FreeRTOS não compila | Ver `lib/FREERTOS_SETUP.md` |
| Bolinha se move sozinha | Calibrar IMU, placa em superfície plana |
| Muito rápido/lento | Ajustar `TILT_SCALE` e `FRICTION` |

---

## 📈 Melhorias Futuras

- [ ] Múltiplos níveis progressivos
- [ ] Highscore salvo em EEPROM
- [ ] Efeitos sonoros (buzzer)
- [ ] Power-ups no labirinto
- [ ] Menu de seleção
- [ ] Animações de transição
- [ ] Calibração do IMU via interface

---

## 🎓 Aprendizados

Este projeto demonstra:
- ✅ Integração de múltiplos periféricos (SPI, I2C, GPIO, UART)
- ✅ Programação com FreeRTOS (tasks, queues, mutex)
- ✅ Física 2D em tempo real
- ✅ Detecção de colisões
- ✅ Renderização gráfica em LCD
- ✅ Processamento de dados de sensores inerciais
- ✅ Máquina de estados para controle de jogo

---

## 📞 Suporte

1. **Compilação**: Verificar `platformio.ini` e FreeRTOS
2. **Hardware**: Conferir conexões e alimentação
3. **Debug**: Monitor serial (115200 baud)
4. **Código**: Comentários detalhados em `src/main.c`

---

## 📜 Licença

Projeto acadêmico - UTFPR Campus Curitiba  
ELF74 - Sistemas Embarcados  
Prof. Eduardo Nunes Dos Santos  
2025

---

**Para começar**: Leia `QUICKSTART.md` 🚀
