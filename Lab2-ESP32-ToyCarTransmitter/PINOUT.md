# ESP32 Pinout Reference for Toy Car Project

## ESP32 DevKit V1 Pinout

```
                    ┌─────────────────┐
                    │     ESP32       │
                    │   Dev Kit V1    │
                    └─────────────────┘
                              │
        ┌─────────────────────┴─────────────────────┐
        │                                           │
EN  │● ○                                         ○ ●│ D23
   VP/36│● ○                                     ○ ●│ D22 ← SCL (MPU6050)
   VN/39│● ○                                     ○ ●│ TX0
D34  │● ○                                        ○ ●│ RX0
   D35  │● ○                                     ○ ●│ D21 ← SDA (MPU6050)
D32  │● ○                                        ○ ●│ D19
   D33  │● ○                                     ○ ●│ D18
   D25  │● ○                                     ○ ●│ D5
   D26  │● ○                                     ○ ●│ D17 → HC-12 RX
   D27  │● ○                                     ○ ●│ D16 ← HC-12 TX
   D14  │● ○                                     ○ ●│ D4
   D12  │● ○                                     ○ ●│ D0
   D13  │● ○                                     ○ ●│ D2  ← LED
    GND │● ○                                     ○ ●│ D15
    VIN │● ○                                     ○ ●│ GND
        └───────────────────────────────────────────┘
        Left Side                    Right Side
```

## Pin Assignments

### I2C (MPU6050)
```
GPIO 21 (SDA) ────────► MPU6050 SDA
GPIO 22 (SCL) ────────► MPU6050 SCL
3.3V          ────────► MPU6050 VCC
GND           ────────► MPU6050 GND
```

### UART2 (HC-12)
```
GPIO 17 (TX2) ────────► HC-12 RX
GPIO 16 (RX2) ────────► HC-12 TX
5V or 3.3V*   ────────► HC-12 VCC
GND           ────────► HC-12 GND
GND           ────────► HC-12 SET (for normal operation)
```
*Check your HC-12 datasheet - some need 5V, others 3.3V

### Status Indicator
```
GPIO 2 ───────────────► Built-in LED
```

## MPU6050 Module Pinout

```
    ┌───────────────────┐
    │    MPU6050        │
    │   GY-521 Module   │
    │                   │
    │  ●─ VCC (3.3V)    │
    │  ●─ GND           │
    │  ●─ SCL (GPIO 22) │
    │  ●─ SDA (GPIO 21) │
    │  ●─ XDA (unused)  │
    │  ●─ XCL (unused)  │
    │  ●─ AD0 (leave floating or GND)
    │  ●─ INT (unused)  │
    └───────────────────┘
```

**I2C Address:**
- AD0 = GND or floating: `0x68` (default)
- AD0 = 3.3V: `0x69`

## HC-12 Module Pinout

```
    ┌───────────────────┐
    │      HC-12        │
    │   433MHz/915MHz   │
    │                   │
    │  ●─ VCC (3.3-5V)  │
    │  ●─ GND           │
    │  ●─ TX  (to ESP32 GPIO 16)
    │  ●─ RX  (to ESP32 GPIO 17)
    │  ●─ SET (to GND for normal operation)
    │                   │
    │      [Antenna]    │
    └───────────────────┘
```

**SET Pin Modes:**
- SET = GND: Normal wireless transmission mode
- SET = 3.3V: AT command configuration mode

## Complete Wiring Table

| Component | Pin | ESP32 Pin | ESP32 GPIO | Notes |
|-----------|-----|-----------|------------|-------|
| **MPU6050** | | | | |
| | VCC | 3.3V | - | NOT 5V! |
| | GND | GND | - | |
| | SDA | D21 | GPIO 21 | I2C Data |
| | SCL | D22 | GPIO 22 | I2C Clock |
| **HC-12** | | | | |
| | VCC | 5V or 3.3V | - | Check datasheet |
| | GND | GND | - | |
| | TX | D16 | GPIO 16 | RX2 |
| | RX | D17 | GPIO 17 | TX2 |
| | SET | GND | - | For normal mode |
| **LED** | | | | |
| | Anode | D2 | GPIO 2 | Built-in LED |

## Power Considerations

### Voltage Levels

```
ESP32:     3.3V logic, 5V tolerant on some pins
MPU6050:   3.3V ONLY (will damage on 5V!)
HC-12:     3.3V-5V (model dependent)
```

### Current Draw

```
Component    | Sleep   | Active  | Peak TX
─────────────┼─────────┼─────────┼─────────
ESP32        | ~5mA    | ~80mA   | ~160mA (WiFi)
MPU6050      | ~8µA    | ~3.5mA  | ~3.5mA
HC-12        | ~16mA   | ~16mA   | ~100mA
─────────────┼─────────┼─────────┼─────────
Total        | ~21mA   | ~100mA  | ~200mA
```

**Recommended Power Supply:**
- **Development:** USB power (5V, 500mA)
- **Toy Car:** 7.4V LiPo → 5V/3.3V regulator (1A+)

## Alternative Pin Assignments

If you need to use different pins (e.g., conflicting with other sensors):

### Alternative I2C Pins
```cpp
// In code, you can use any GPIO for I2C:
#define I2C_SDA  25  // Any GPIO
#define I2C_SCL  26  // Any GPIO

Wire.begin(I2C_SDA, I2C_SCL);
```

### Alternative UART Pins
```cpp
// UART2 can be remapped to any GPIO:
#define HC12_RX  4   // Any GPIO
#define HC12_TX  5   // Any GPIO

HC12.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);
```

**Avoid these pins:**
- GPIO 6-11: Connected to flash (don't use!)
- GPIO 0: Boot mode pin (risky)
- GPIO 1, 3: Serial debug (USB)

## Visual Connection Guide

```
         ┌──────────────────────────────────────────┐
         │            ESP32 Dev Board                │
         │                                           │
         │   3.3V ─────┬──────────► MPU6050 VCC     │
         │             │                             │
         │   GPIO 21 ──┼──────────► MPU6050 SDA     │
         │   GPIO 22 ──┼──────────► MPU6050 SCL     │
         │             │                             │
         │   5V ───────┼──────────► HC-12 VCC       │
         │   GPIO 17 ──┼──────────► HC-12 RX        │
         │   GPIO 16 ──┼──────────► HC-12 TX        │
         │             │                             │
         │   GND ──────┴──┬───────► MPU6050 GND     │
         │                ├───────► HC-12 GND       │
         │                └───────► HC-12 SET       │
         └──────────────────────────────────────────┘
```

## Testing Individual Components

### Test 1: LED Blink
```cpp
void setup() {
    pinMode(2, OUTPUT);
}
void loop() {
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    delay(500);
}
```
✅ LED should blink every second

### Test 2: I2C Scanner (Find MPU6050)
```cpp
void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C device at 0x%02X\n", addr);
        }
    }
}
```
✅ Should find MPU6050 at 0x68

### Test 3: HC-12 Echo
```cpp
HardwareSerial HC12(2);

void setup() {
    Serial.begin(115200);
    HC12.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
    if (HC12.available()) {
        Serial.write(HC12.read());
    }
    if (Serial.available()) {
        HC12.write(Serial.read());
    }
}
```
✅ Type in Serial Monitor → Should transmit via HC-12

## Multimeter Checks

Before powering on:

1. **Continuity Check:**
   - GND to GND: Should beep
   - 3.3V to GND: Should NOT beep
   - 5V to GND: Should NOT beep

2. **Voltage Check (after power on):**
   - Measure 3.3V pin: Should read 3.2-3.4V
   - Measure 5V pin: Should read 4.8-5.2V
   - Measure MPU6050 VCC: Should read 3.3V (NOT 5V!)

3. **I2C Pullup Check:**
   - SDA to 3.3V: Should read ~3.3V (pulled up)
   - SCL to 3.3V: Should read ~3.3V (pulled up)

## Common Mistakes to Avoid

❌ **MPU6050 VCC to 5V** → Will damage sensor!
❌ **HC-12 TX to ESP32 TX** → Should be TX to RX!
❌ **SET pin floating** → HC-12 won't work properly
❌ **No common ground** → Erratic behavior
❌ **Antenna removed** → Very short range
❌ **Wrong I2C pins** → Sensor not found

## Safety Notes

⚠️ **Never connect MPU6050 to 5V!** Use 3.3V only.
⚠️ **HC-12 can draw 100mA** during transmission - ensure adequate power supply
⚠️ **Check HC-12 voltage rating** - some are 3.3V, some are 5V
⚠️ **Don't connect GPIO directly to high voltage** - use level shifters if needed

Happy wiring! 🔌
