# Quick Start - Simulador Digital de Labirinto

## 1. Setup Rápido

### Passo 1: FreeRTOS-Kernel
```bash
cd lib
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd FreeRTOS-Kernel
git checkout V10.5.1
cd ../..
```

**OU** copie do Lab2_RTOS:
```bash
xcopy "..\Lab2_RTOS\lib\FreeRTOS-Kernel" "lib\FreeRTOS-Kernel\" /E /I /Y
```

### Passo 2: Compilar
```bash
pio run
```

### Passo 3: Upload
```bash
pio run -t upload
```

### Passo 4: Monitor (opcional)
```bash
pio device monitor
```

## 2. Como Jogar

1. **Inicie o jogo**: O sistema inicializa automaticamente
2. **Controle a bolinha**: Incline a placa Black Pill
   - Frente/Trás: Eixo Y do acelerômetro
   - Esquerda/Direita: Eixo X do acelerômetro
3. **Objetivo**: Levar a bolinha até a célula verde (canto inferior direito)
4. **Evite**: Não caia nos buracos (células pretas)
5. **Vidas**: Você tem 3 vidas
6. **Reset**: Pressione o botão (PA0) para reiniciar

## 3. Conexões Hardware

### STM32F411 (Black Pill)

#### Display ST7789 (SPI1)
- **VCC** → 3.3V
- **GND** → GND
- **SCL** → PA5 (SPI1_SCK)
- **SDA** → PA7 (SPI1_MOSI)
- **RES** → PB0 (Reset)
- **DC**  → PB1 (Data/Command)
- **CS**  → PA4 (Chip Select)
- **BLK** → 3.3V (Backlight)

#### MPU6050 (I2C1)
- **VCC** → 3.3V
- **GND** → GND
- **SCL** → PB8 (I2C1_SCL)
- **SDA** → PB9 (I2C1_SDA)
- **AD0** → GND (Endereço 0x68)

#### Botão
- **BTN** → PA0
- **GND** → GND
- *Obs*: Pull-up interno habilitado

#### Debug UART
- **TX** → PA2 (USART2_TX)
- **RX** → PA3 (USART2_RX)

## 4. Ajustes de Calibração

Se a bolinha estiver se movendo muito rápido ou devagar, ajuste em `main.c`:

```c
// Mais sensível: aumentar valor
#define TILT_SCALE      0.0015f  // tente 0.002f ou 0.0025f

// Menos atrito: diminuir valor
#define FRICTION        0.92f    // tente 0.90f ou 0.88f

// Velocidade máxima
#define MAX_VELOCITY    80.0f    // tente 100.0f ou 120.0f
```

Recompile após alterações:
```bash
pio run -t upload
```

## 5. Troubleshooting

### Erro: MPU6050 não detectado
✓ Verificar conexões I2C
✓ Testar com I2C scanner
✓ Verificar alimentação 3.3V

### Erro: Display não acende
✓ Verificar conexões SPI
✓ Verificar backlight (BLK → 3.3V)
✓ Verificar alimentação

### Erro: FreeRTOS não compila
✓ Verificar se `lib/FreeRTOS-Kernel` existe
✓ Seguir `lib/FREERTOS_SETUP.md`

### Bolinha se move sozinha
✓ MPU6050 precisa de calibração
✓ Colocar placa em superfície plana ao ligar
✓ Aguardar 2 segundos após boot

## 6. Estrutura de Arquivos

```
ProjetoFinal_Labirinto/
├── platformio.ini          # Configuração PlatformIO
├── README.md               # Informações gerais
├── DOCUMENTATION.md        # Documentação detalhada
├── QUICKSTART.md          # Este arquivo
├── include/               # Headers
│   ├── board.h
│   ├── delay_rtos.h
│   ├── font5x7.h
│   ├── FreeRTOSConfig.h
│   ├── main.h
│   ├── mpu6050.h
│   ├── serial_stdio.h
│   └── st7789.h
├── src/                   # Código fonte
│   ├── main.c            # ★ Lógica principal do jogo
│   ├── delay_rtos.c
│   ├── heap_4.c
│   ├── mpu6050.c
│   ├── serial_stdio.c
│   └── st7789.c
└── lib/
    └── FreeRTOS-Kernel/  # Biblioteca FreeRTOS
```

## 7. Próximos Passos

Após testar o jogo básico, você pode:

1. **Modificar o labirinto**: Edite `maze_map` em `main.c`
2. **Criar novos níveis**: Adicione mais mapas
3. **Ajustar dificuldade**: Mude parâmetros de física
4. **Adicionar features**: Score, timer, power-ups

Consulte `DOCUMENTATION.md` para detalhes técnicos completos.

## 8. Suporte

Para dúvidas ou problemas:
- Verificar mensagens no monitor serial (115200 baud)
- Consultar documentação detalhada
- Revisar código de exemplo no Lab2_RTOS

---

**Bom jogo! 🎮**
