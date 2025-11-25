Diagrama de ligações — BlackPill (STM32F411CE)

Resumo dos pinos usados no projeto:

- ST7789 (240x240 TFT):
  - SCK  -> `PA5`  (SPI1_SCK)
  - MOSI -> `PA7`  (SPI1_MOSI)
  - DC   -> `PB0`
  - RST  -> `PB1`
  - BL   -> `PB6`  (backlight)
  - CS   -> `PB10` (opcional; mantido LOW no firmware)
  - VCC  -> `3.3V`
  - GND  -> `GND`

- MPU6050 (I2C IMU):
  - SCL  -> `PB8`  (I2C1_SCL, AF4)
  - SDA  -> `PB9`  (I2C1_SDA, AF4)
  - AD0  -> `GND`  (endereço I2C = 0x68)
  - VCC  -> `3.3V`
  - GND  -> `GND`

- Serial (USB-UART TTL adapter):
  - MCU TX -> `PA9` (conectar ao RX do adaptador)
  - MCU RX -> `PA10`(conectar ao TX do adaptador)
  - GND     -> `GND`

- Programação / Debug (ST-Link SWD):
  - SWDIO -> `PA13`
  - SWCLK -> `PA14`
  - 3.3V  -> `3.3V`
  - GND   -> `GND`

Notas e recomendações:

- I2C (PB8/PB9) é configurado em open-drain com pull-ups internas ativas no código; se enfrentar instabilidade, adicione resistores pull-up externos (4.7kΩ a 10kΩ) entre SCL/SDA e 3.3V.
- O módulo ST7789 deve ser alimentado em 3.3V (não usar 5V diretamente) — a BlackPill também fornece 3.3V via regulador da placa quando alimentada corretamente.
- CS do LCD está sendo forçado LOW no firmware; se usar outro display com CS ativo, adapte o pino ou a lógica.
- Se o adaptador USB-UART fornecer 5V no pino VCC, NÃO conecte ao 3.3V do MCU — use somente GND e os sinais TTL (TX/RX), e alimente o display/IMU com 3.3V.

Arquivos úteis gerados:
- `wiring_diagram.svg` — diagrama gráfico das ligações (na raiz do projeto)
- `WIRING.md` — este resumo legível

Se quiser, eu posso:
- Gerar versão PNG do `wiring_diagram.svg` para facilitar visualização;
- Ajustar o diagrama trocando pinos ou adicionando um conector JST para o display;
- Gerar PDF com instruções de montagem passo-a-passo.
