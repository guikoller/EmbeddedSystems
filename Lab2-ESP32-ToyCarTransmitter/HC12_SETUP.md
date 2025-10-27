# HC-12 Configuration Guide

Complete guide for configuring HC-12 wireless modules.

## 📡 What is HC-12?

HC-12 is a wireless serial port communication module with:
- **Frequency:** 433MHz or 915MHz (ISM band)
- **Range:** Up to 1800m in open air (typical: 100-600m)
- **Power:** 100mW transmit power
- **Baud Rate:** 1200-115200 bps
- **Channels:** 100 channels (433.4-473.6 MHz)
- **Modes:** FU1-FU4 (different power/range/speed)

## 🔧 AT Command Mode

### Entering AT Mode

**Method 1: Hardware (Recommended)**
1. Disconnect SET pin from GND
2. Connect SET pin to 3.3V (or HIGH GPIO)
3. Wait 40ms
4. Module is now in AT mode
5. LED will blink slowly (~1Hz)

**Method 2: Code**
```cpp
#define HC12_SET_PIN 5  // Use any available GPIO

void setup() {
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, HIGH);  // Enter AT mode
    delay(100);
    
    HC12.begin(9600);
    HC12.println("AT");  // Send AT command
    
    delay(100);
    digitalWrite(HC12_SET_PIN, LOW);   // Exit AT mode
}
```

### Exiting AT Mode

1. Connect SET pin back to GND (or LOW GPIO)
2. Wait 80ms
3. Module returns to normal transmission mode
4. LED will blink rapidly during TX

## 📋 AT Command Reference

### Basic Commands

| Command | Description | Response | Example |
|---------|-------------|----------|---------|
| `AT` | Test connection | `OK` | Test if module responds |
| `AT+RX` | Query all settings | Shows config | See current setup |
| `AT+DEFAULT` | Reset to factory | `OK` | Restore defaults |
| `AT+SLEEP` | Enter sleep mode | `OK+SLEEP` | Low power mode |

### Baud Rate Commands

| Command | Baud Rate | Response |
|---------|-----------|----------|
| `AT+B1200` | 1200 | `OK+B1200` |
| `AT+B2400` | 2400 | `OK+B2400` |
| `AT+B4800` | 4800 | `OK+B4800` |
| `AT+B9600` | 9600 (default) | `OK+B9600` |
| `AT+B19200` | 19200 | `OK+B19200` |
| `AT+B38400` | 38400 | `OK+B38400` |
| `AT+B57600` | 57600 | `OK+B57600` |
| `AT+B115200` | 115200 | `OK+B115200` |

**Note:** Air baud rate ≠ serial baud rate. See modes below.

### Channel Commands

| Command | Frequency | Response |
|---------|-----------|----------|
| `AT+C001` | 433.4 MHz (default) | `OK+C001` |
| `AT+C020` | 434.3 MHz | `OK+C020` |
| `AT+C050` | 437.2 MHz | `OK+C050` |
| `AT+C100` | 473.6 MHz | `OK+C100` |

**Formula:** Frequency (MHz) = 433.4 + (Channel - 1) × 0.4

**Choose channels far apart for multiple systems:**
- System 1: Channel 001 (433.4 MHz)
- System 2: Channel 050 (437.2 MHz)
- System 3: Channel 100 (473.6 MHz)

### Transmission Mode Commands

| Mode | Command | Power | Range | Air Rate | Current | Best For |
|------|---------|-------|-------|----------|---------|----------|
| FU1 | `AT+FU1` | -1dBm | Short | 236 kbps | ~3.6mA | Battery saving |
| FU2 | `AT+FU2` | 2dBm | Medium | 4.8 kbps | ~80mA | Balanced |
| FU3 | `AT+FU3` | 8dBm | Long | 4.8 kbps | ~100mA | **Default, Recommended** |
| FU4 | `AT+FU4` | 20dBm | Max | 0.5 kbps | ~100mA | Maximum range |

**Response:** `OK+FU3` (for FU3)

**Recommendation:** Use **FU3** for best balance of range/speed/reliability.

## 🚀 Quick Configuration Scripts

### Standard Setup (Recommended)

```cpp
// Use this for most applications
void configureHC12Standard() {
    pinMode(HC12_SET_PIN, OUTPUT);
    
    // Enter AT mode
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.begin(9600);  // Temporary baud for config
    
    // Configure
    HC12.println("AT+B9600");   // 9600 baud
    delay(100);
    HC12.println("AT+C001");    // Channel 1
    delay(100);
    HC12.println("AT+FU3");     // Max reliable power
    delay(100);
    
    // Verify
    HC12.println("AT+RX");
    delay(200);
    while (HC12.available()) {
        Serial.write(HC12.read());
    }
    
    // Exit AT mode
    digitalWrite(HC12_SET_PIN, LOW);
    delay(100);
    
    Serial.println("\nHC-12 configured!");
}
```

### High-Speed Setup

```cpp
// For maximum data rate
void configureHC12HighSpeed() {
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.begin(9600);
    
    HC12.println("AT+B115200");  // High baud
    delay(100);
    HC12.println("AT+C001");
    delay(100);
    HC12.println("AT+FU1");      // Fastest air rate
    delay(100);
    
    digitalWrite(HC12_SET_PIN, LOW);
    delay(100);
    
    // Now use 115200 for communication
    HC12.begin(115200, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
}
```

### Long-Range Setup

```cpp
// For maximum distance
void configureHC12LongRange() {
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.begin(9600);
    
    HC12.println("AT+B9600");
    delay(100);
    HC12.println("AT+C001");    // Lower freq = better propagation
    delay(100);
    HC12.println("AT+FU4");     // Maximum power
    delay(100);
    
    digitalWrite(HC12_SET_PIN, LOW);
    delay(100);
}
```

## 🔍 Querying Settings

### Get All Settings

```cpp
// Enter AT mode
digitalWrite(HC12_SET_PIN, HIGH);
delay(100);

HC12.println("AT+RX");
delay(200);

// Read response
while (HC12.available()) {
    Serial.write(HC12.read());
}

digitalWrite(HC12_SET_PIN, LOW);
```

**Example Response:**
```
OK+B9600
OK+C001
OK+FU3
```

### Parse Settings in Code

```cpp
String getBaudRate() {
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.println("AT+RX");
    delay(200);
    
    String response = "";
    while (HC12.available()) {
        response += (char)HC12.read();
    }
    
    digitalWrite(HC12_SET_PIN, LOW);
    
    // Parse response for baud rate
    int baudIndex = response.indexOf("OK+B");
    if (baudIndex >= 0) {
        String baud = response.substring(baudIndex + 4, baudIndex + 10);
        return baud;
    }
    
    return "Unknown";
}
```

## 🧪 Testing HC-12 Modules

### Test 1: Single Module Echo Test

```cpp
// Upload to one ESP32 with HC-12
void setup() {
    Serial.begin(115200);
    HC12.begin(9600, SERIAL_8N1, 16, 17);
    
    // Enter AT mode
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    Serial.println("Type AT commands:");
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        HC12.println(cmd);
        delay(100);
        
        while (HC12.available()) {
            Serial.write(HC12.read());
        }
    }
}
```

### Test 2: Two-Module Range Test

**Transmitter Code:**
```cpp
void loop() {
    static int counter = 0;
    
    HC12.print("Packet #");
    HC12.println(counter++);
    
    Serial.print("Sent: ");
    Serial.println(counter - 1);
    
    delay(1000);
}
```

**Receiver Code:**
```cpp
void loop() {
    if (HC12.available()) {
        String received = HC12.readStringUntil('\n');
        Serial.println(received);
        
        // Blink LED
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
    }
}
```

Walk apart and watch when packets stop arriving = max range!

## 📊 Performance vs. Mode

| Mode | Data Rate | Range | Latency | Battery Life |
|------|-----------|-------|---------|--------------|
| FU1 | ★★★★★ | ★☆☆☆☆ | ★★★★★ | ★★★★★ |
| FU2 | ★★★☆☆ | ★★★☆☆ | ★★★☆☆ | ★★★☆☆ |
| FU3 | ★★★☆☆ | ★★★★☆ | ★★★☆☆ | ★★☆☆☆ |
| FU4 | ★☆☆☆☆ | ★★★★★ | ★☆☆☆☆ | ★★☆☆☆ |

**For toy car IMU:** Use **FU3** (good balance)

## 🐛 Troubleshooting

### No "OK" Response

**Possible causes:**
- ✅ SET pin not at 3.3V (must be HIGH for AT mode)
- ✅ Wrong baud rate (try 1200, 2400, 4800, 9600)
- ✅ TX/RX swapped
- ✅ Not waiting long enough (40ms min after SET HIGH)

**Solution:**
```cpp
// Try all common baud rates
int baudRates[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

for (int i = 0; i < 8; i++) {
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.begin(baudRates[i], SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
    HC12.println("AT");
    delay(200);
    
    if (HC12.available()) {
        Serial.print("Found at baud: ");
        Serial.println(baudRates[i]);
        break;
    }
    
    digitalWrite(HC12_SET_PIN, LOW);
    delay(100);
}
```

### Modules Won't Communicate

**Checklist:**
- [ ] Both modules on same channel
- [ ] Both modules at same baud rate  
- [ ] Both modules in same mode (FU1-4)
- [ ] SET pins connected to GND (normal mode)
- [ ] Antennas attached
- [ ] Power supply adequate (100mA needed)
- [ ] Within range (start close, then separate)

### Unreliable Transmission

**Possible issues:**
1. **Interference:** Move away from WiFi routers, motors, ESCs
2. **Antenna orientation:** Keep vertical for best range
3. **Power supply:** Add capacitors, ensure stable voltage
4. **Mode mismatch:** Both must use same FU mode

**Solutions:**
```cpp
// Add checksum to data
char data[64];
sprintf(data, "%s %.2f, %.2f, %.2f, %.2f, %.2f, %.2f|%02X\n",
        HEADER, ax, ay, az, gx, gy, gz, calculateChecksum());

// Or use simple packet counter
static int packetID = 0;
sprintf(data, "[%04d] %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
        packetID++, ax, ay, az, gx, gy, gz);
```

## 🌐 Multi-Channel Configuration

To run multiple systems without interference:

**System 1 (Red Car):**
```cpp
HC12.println("AT+C001");  // 433.4 MHz
```

**System 2 (Blue Car):**
```cpp
HC12.println("AT+C050");  // 437.2 MHz
```

**System 3 (Green Car):**
```cpp
HC12.println("AT+C100");  // 473.6 MHz
```

Minimum 20-channel spacing recommended for no interference.

## 💾 Save Configuration

HC-12 saves configuration automatically to EEPROM.

**To verify after power cycle:**
```cpp
void verifyConfig() {
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(100);
    
    HC12.println("AT+RX");
    delay(200);
    
    Serial.println("Saved configuration:");
    while (HC12.available()) {
        Serial.write(HC12.read());
    }
    
    digitalWrite(HC12_SET_PIN, LOW);
}
```

## 📝 Configuration Worksheet

Use this when configuring your modules:

```
Module #1 (Transmitter):
└─ Baud Rate: _______________
└─ Channel:   _______________
└─ Mode:      _______________
└─ Verified:  [ ]

Module #2 (Receiver):
└─ Baud Rate: _______________
└─ Channel:   _______________
└─ Mode:      _______________
└─ Verified:  [ ]

Settings Match: [ ]
Test TX→RX:     [ ]
Test RX→TX:     [ ]
Range Test:     ___________ meters
```

## 🔗 Additional Resources

- [HC-12 Datasheet](http://www.hc01.com/downloads)
- [AT Command Full Reference](https://statics3.seeedstudio.com/assets/file/bazaar/product/HC-12_english_datasheets.pdf)
- [Forum Discussions](https://forum.arduino.cc/t/hc-12-wireless-serial-port-communication-module/173980)

Happy configuring! 📡
