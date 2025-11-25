# Troubleshooting Guide - Simulador de Labirinto

Este guia ajuda a resolver problemas comuns durante o desenvolvimento e execução do projeto.

---

## 🔴 Problemas de Compilação

### Erro: "FreeRTOS.h: No such file or directory"

**Causa**: FreeRTOS-Kernel não está instalado em `lib/FreeRTOS-Kernel/`

**Solução**:
```bash
cd lib
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd FreeRTOS-Kernel
git checkout V10.5.1
```

**Alternativa**: Copiar do Lab2_RTOS (se disponível)
```bash
xcopy "..\Lab2_RTOS\lib\FreeRTOS-Kernel" "lib\FreeRTOS-Kernel\" /E /I /Y
```

---

### Erro: "undefined reference to `vTaskStartScheduler`"

**Causa**: Arquivos do FreeRTOS não estão sendo compilados

**Solução**:
1. Verificar se `lib/FreeRTOS-Kernel/` existe e tem conteúdo
2. Verificar `platformio.ini` contém:
   ```ini
   build_flags =
     -Ilib/FreeRTOS-Kernel/include
     -Ilib/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
   ```
3. Limpar e recompilar:
   ```bash
   pio run -t clean
   pio run
   ```

---

### Erro: Linker errors com float/double

**Causa**: Flags de FPU incorretas

**Solução**: Verificar em `platformio.ini`:
```ini
build_flags =
  -mfloat-abi=softfp
  -mfpu=fpv4-sp-d16
```

---

## 🔴 Problemas de Hardware

### MPU6050 não é detectado ("MPU6050 INIT FAILED")

**Sintomas**: Display mostra "MPU6050 ERROR" em vermelho

**Verificações**:

1. **Conexões I2C**:
   - SDA → PB9
   - SCL → PB8
   - VCC → 3.3V (NÃO 5V!)
   - GND → GND

2. **Endereço I2C**:
   - AD0 → GND = endereço 0x68
   - AD0 → VCC = endereço 0x69
   - Verificar no código (padrão: 0x68)

3. **Teste de continuidade**:
   - Usar multímetro para verificar conexões
   - Verificar solda dos pinos

4. **I2C Scanner** (código de teste):
   ```c
   for (uint8_t addr = 0x00; addr < 0x7F; addr++) {
       // Testar cada endereço I2C
       // Imprimir endereços que respondem
   }
   ```

**Solução comum**: Trocar SDA/SCL (inversão acidental)

---

### Display não acende ou mostra apenas branco

**Sintomas**: Backlight aceso mas sem imagem / Tela completamente preta

**Verificações**:

1. **Conexões SPI**:
   - SCL → PA5 (SPI1_SCK)
   - SDA → PA7 (SPI1_MOSI)
   - CS → PA4
   - DC → PB1
   - RES → PB0
   - BLK → 3.3V
   - VCC → 3.3V
   - GND → GND

2. **Alimentação**:
   - Verificar 3.3V no pino VCC
   - Corrente suficiente (display consome ~20-50mA)

3. **Reset manual**:
   - Desconectar RES temporariamente
   - Religar após 1 segundo

4. **Teste básico**:
   ```c
   st7789_fill_screen_dma(COLOR_RED);
   delay_ms(1000);
   st7789_fill_screen_dma(COLOR_BLUE);
   ```

---

### Botão não funciona

**Sintomas**: Pressionar botão não reinicia o jogo

**Verificações**:

1. **Conexões**:
   - Botão entre PA0 e GND
   - Pull-up interno está habilitado no código

2. **Teste de leitura**:
   ```c
   uint8_t btn = button_read();
   printf("Button state: %d\n", btn);
   // 1 = não pressionado (pull-up)
   // 0 = pressionado
   ```

3. **Debounce**:
   - Código já tem debounce de 20ms
   - Botão pode estar com "bounce" mecânico

---

## 🔴 Problemas de Comportamento

### Bolinha se move sozinha (sem inclinar)

**Causa**: Offset do MPU6050 / Calibração

**Solução temporária**: Colocar placa em superfície plana e PERFEITAMENTE nivelada ao ligar

**Solução permanente**: Adicionar calibração automática no código:
```c
// Ler 100 amostras e calcular offset
int32_t ax_offset = 0, ay_offset = 0;
for (int i = 0; i < 100; i++) {
    mpu6050_raw_t data;
    mpu6050_read_all(&data);
    ax_offset += data.ax;
    ay_offset += data.ay;
    delay_ms(10);
}
ax_offset /= 100;
ay_offset /= 100;

// Subtrair offset ao usar:
float tilt_x = (float)(imu_data.ay - ay_offset);
float tilt_y = (float)(imu_data.ax - ax_offset);
```

---

### Física muito rápida / Bolinha incontrolável

**Causa**: Parâmetros de física desajustados

**Solução**: Ajustar em `main.c`:

```c
// REDUZIR sensibilidade:
#define TILT_SCALE 0.001f  // padrão: 0.0015f

// AUMENTAR atrito:
#define FRICTION 0.95f     // padrão: 0.92f

// REDUZIR velocidade máxima:
#define MAX_VELOCITY 60.0f // padrão: 80.0f
```

---

### Física muito lenta / Bolinha quase não se move

**Causa**: Parâmetros de física muito conservadores

**Solução**: Ajustar em `main.c`:

```c
// AUMENTAR sensibilidade:
#define TILT_SCALE 0.0025f // padrão: 0.0015f

// REDUZIR atrito:
#define FRICTION 0.88f     // padrão: 0.92f

// AUMENTAR velocidade máxima:
#define MAX_VELOCITY 120.0f // padrão: 80.0f
```

---

### Bolinha atravessa paredes

**Causa**: Detecção de colisão com problemas

**Debug**:
```c
// Adicionar prints na função ball_update_physics()
if (collision) {
    printf("Collision at x=%.1f y=%.1f\n", new_x, new_y);
}
```

**Soluções**:
1. Verificar constante `BALL_RADIUS` (padrão: 4)
2. Aumentar número de pontos de checagem (8 → 16):
   ```c
   for (int angle = 0; angle < 360; angle += 22) {  // era 45
   ```
3. Reduzir velocidade máxima

---

### Display pisca / Renderização lenta

**Causa**: Muitas operações de desenho por frame

**Solução**:
1. Já otimizado com DMA (`st7789_fill_screen_dma`)
2. Reduzir taxa de atualização:
   ```c
   // Em Display_Task()
   vTaskDelay(pdMS_TO_TICKS(100)); // era 50ms (20 FPS → 10 FPS)
   ```
3. Não redesenhar o labirinto inteiro a cada frame (otimização avançada)

---

### Stack Overflow / Malloc Failed

**Sintomas**: Sistema trava em `vApplicationStackOverflowHook` ou `vApplicationMallocFailedHook`

**Causa**: Memória insuficiente para as tasks

**Solução**: Aumentar heap do FreeRTOS em `FreeRTOSConfig.h`:
```c
#define configTOTAL_HEAP_SIZE ((size_t)(40 * 1024)) // 40KB
```

Ou reduzir tamanho das stacks das tasks em `main.c`:
```c
xTaskCreate(IMU_Task,       "IMU",   128, NULL, 3, NULL); // era 256
xTaskCreate(GameLogic_Task, "LOGIC", 384, NULL, 2, NULL); // era 512
xTaskCreate(Display_Task,   "DISP",  384, NULL, 1, NULL); // era 512
```

---

## 🔴 Problemas de Debug

### Monitor serial não mostra nada

**Verificações**:
1. Baud rate correto (115200)
2. Porta COM correta
3. Cabo USB-Serial conectado corretamente:
   - TX (PA2) → RX do conversor
   - RX (PA3) → TX do conversor
   - GND comum

**Teste**:
```bash
pio device monitor -b 115200
```

---

### Hard Fault / Sistema trava

**Debug**:
1. Adicionar prints antes de operações críticas
2. Usar LED para indicar onde travou:
   ```c
   // Piscar LED em diferentes frequências
   // para identificar a seção do código
   ```

**Causas comuns**:
- Divisão por zero
- Acesso a ponteiro NULL
- Stack overflow
- Array out of bounds

---

## 🔴 Problemas Específicos do Jogo

### Sempre cai no buraco mesmo longe dele

**Debug**: Verificar função `ball_check_hole()`:
```c
int cell_x = (int)((ball.x - MAZE_OFFSET_X) / CELL_SIZE);
int cell_y = (int)((ball.y - MAZE_OFFSET_Y) / CELL_SIZE);
printf("Ball at cell (%d,%d) = %d\n", cell_x, cell_y, 
       maze_get_cell(cell_x, cell_y));
```

**Causa possível**: Offset ou escala errados

---

### Não detecta vitória no objetivo

**Verificação**: 
1. Coordenadas do objetivo em `maze_init()`:
   ```c
   maze.goal_x = 14;
   maze.goal_y = 14;
   ```
2. Célula marcada como `CELL_GOAL` (3) no mapa
3. Testar função `ball_check_goal()` com prints

---

### Timer não funciona corretamente

**Causa**: Overflow de tick count do FreeRTOS

**Solução**: Já implementado corretamente, mas verificar:
```c
game_time_ms = (xTaskGetTickCount() - start_ticks) * portTICK_PERIOD_MS;
```

---

## 📞 Suporte Adicional

Se o problema persistir:

1. **Limpar projeto**:
   ```bash
   pio run -t clean
   rm -rf .pio
   pio run
   ```

2. **Verificar versões**:
   - PlatformIO: Atualizar para versão mais recente
   - FreeRTOS: Usar versão V10.5.1

3. **Documentação**:
   - Consultar `DOCUMENTATION.md` para detalhes técnicos
   - Revisar código comentado em `src/main.c`

4. **Hardware**:
   - Testar componentes individualmente
   - Verificar com multímetro (continuidade, tensões)

5. **Código de referência**:
   - Comparar com Lab2_RTOS (funcionando)
   - Testar drivers isoladamente

---

## ✅ Checklist de Diagnóstico

Antes de pedir ajuda, verificar:

- [ ] FreeRTOS-Kernel está em `lib/FreeRTOS-Kernel/`
- [ ] Compilação sem erros ou warnings
- [ ] Upload concluído com sucesso
- [ ] Todas as conexões verificadas com multímetro
- [ ] Alimentação 3.3V estável
- [ ] Monitor serial funcionando (115200 baud)
- [ ] Mensagens de inicialização aparecem no serial
- [ ] MPU6050 detectado ("MPU6050 OK")
- [ ] Display mostra algo (mesmo que errado)
- [ ] Botão fisicamente conectado

---

**Boa sorte! 🍀**

Se encontrar um problema não listado aqui, adicione a solução para ajudar outros!
