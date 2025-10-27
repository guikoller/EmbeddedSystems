# ESP32 Toy Car IMU Transmitter with HC-12 RF Module

Complete wireless IMU data transmission system for toy car applications.

## 🎯 System Overview

```
┌─────────────────────────────┐         ┌─────────────────────────────┐
│   TOY CAR (Transmitter)     │         │   RECEIVER STATION          │
│                             │         │                             │
│  ┌──────────┐  ┌─────────┐ │         │  ┌─────────┐  ┌──────────┐ │
│  │ MPU6050  │  │  ESP32  │ │  ~~~~   │  │  ESP32  │  │   PC     │ │
│  │   IMU    ├──┤         ├─┤  RF TX  ├──┤         ├──┤ Python   │ │
│  └──────────┘  │         │ │  ~~~~   │  │         │  │ Plotter  │ │
│       I2C      │  UART2  │ │         │  │  UART2  │  │          │ │
│                └────┬────┘ │         │  └────┬────┘  └──────────┘ │
│                     │      │         │       │                     │
│                ┌────▼────┐ │         │  ┌────▼────┐                │
│                │  HC-12  │ │         │  │  HC-12  │                │
│                │   TX    │ │         │  │   RX    │                │
│                └─────────┘ │         │  └─────────┘                │
└─────────────────────────────┘         └─────────────────────────────┘
```

## 📦 Hardware Requirements

### Toy Car (Transmitter)
- 1x ESP32 Dev Board
- 1x MPU6050 IMU Sensor (6-axis: accelerometer + gyroscope)
- 1x HC-12 RF Module (433MHz or 915MHz)
- 1x Battery (e.g., 7.4V LiPo for toy car)
- 1x Voltage regulator (if needed for 3.3V/5V)
- Jumper wires

### Receiver Station
- **Option 1:** 1x HC-12 RF Module + USB-to-Serial adapter (e.g., FTDI, CP2102)
- **Option 2:** 1x ESP32 + HC-12 (acts as USB bridge)

## 🔌 Wiring Diagrams

### Transmitter (Toy Car)

```
ESP32 Dev Board          MPU6050 IMU
┌─────────────────┐      ┌─────────────┐
│                 │      │             │
│ 3.3V ───────────┼──────┤ VCC         │
│ GND ────────────┼──────┤ GND         │
│ GPIO 21 (SDA) ──┼──────┤ SDA         │
│ GPIO 22 (SCL) ──┼──────┤ SCL         │
│                 │      └─────────────┘
│                 │
│                 │      HC-12 Module
│                 │      ┌─────────────┐
│ 5V* ────────────┼──────┤ VCC         │
│ GND ────────────┼──────┤ GND         │
│ GPIO 17 (TX2) ──┼──────┤ RX          │
│ GPIO 16 (RX2) ──┼──────┤ TX          │
│ GND ────────────┼──────┤ SET         │
└─────────────────┘      └─────────────┘

* Some HC-12 modules need 5V, others work with 3.3V
  Check your module's datasheet!
```

### Receiver Station (Option 1: ESP32 Bridge)

```
ESP32 Dev Board          HC-12 Module          PC
┌─────────────────┐      ┌─────────────┐      ┌──────────┐
│                 │      │             │      │          │
│ 5V* ────────────┼──────┤ VCC         │      │          │
│ GND ────────────┼──────┤ GND         │      │          │
│ GPIO 17 (TX2) ──┼──────┤ RX          │      │          │
│ GPIO 16 (RX2) ──┼──────┤ TX          │      │  Python  │
│ GND ────────────┼──────┤ SET         │      │  Plotter │
│                 │      └─────────────┘      │          │
│ USB Port ───────┼───────── USB Cable ───────┤ COM Port │
└─────────────────┘                           └──────────┘
```

### Receiver Station (Option 2: Direct USB-Serial)

```
HC-12 Module            USB-to-Serial          PC
┌─────────────┐         ┌─────────────┐      ┌──────────┐
│             │         │             │      │          │
│ VCC ────────┼─────────┤ 5V          │      │          │
│ GND ────────┼─────────┤ GND         │      │  Python  │
│ TX ─────────┼─────────┤ RX          │      │  Plotter │
│ RX ─────────┼─────────┤ TX          │      │          │
│ SET ────────┼─── GND  │             │      │          │
└─────────────┘         │ USB Port ───┼──────┤ COM Port │
                        └─────────────┘      └──────────┘
```

## ⚙️ Software Setup

### 1. Install PlatformIO

**VS Code:**
1. Install VS Code
2. Install PlatformIO extension
3. Restart VS Code

**Command Line:**
```bash
pip install platformio
```

### 2. Build and Upload Transmitter Code

```bash
# Navigate to project folder
cd Lab2-ESP32-ToyCarTransmitter

# Build the project
pio run

# Upload to ESP32 (make sure ESP32 is connected via USB)
pio run --target upload

# Monitor serial output (optional, for debugging)
pio device monitor
```

**Via VS Code:**
1. Open folder: `Lab2-ESP32-ToyCarTransmitter`
2. Click PlatformIO icon (alien head) in sidebar
3. Click "Build" (checkmark icon)
4. Connect ESP32 via USB
5. Click "Upload" (arrow icon)

### 3. Setup Receiver

#### Option 1: ESP32 Bridge (Recommended)

```bash
# Edit platformio.ini to create receiver environment
# Or rename receiver.cpp.example to main.cpp

# Upload receiver code to second ESP32
pio run --target upload
```

#### Option 2: Direct HC-12 to USB-Serial

No code needed! Just wire HC-12 to USB-Serial adapter.

### 4. Configure HC-12 Modules (One-time setup)

HC-12 modules need to be on the same channel and settings.

**To enter AT command mode:**
1. Connect SET pin to GND → Disconnect from GND → Connect to 3.3V
2. OR use a spare GPIO and toggle it HIGH

**Common AT Commands:**
```
AT                  // Test communication (returns "OK")
AT+B9600           // Set baud rate to 9600
AT+C001            // Set channel to 001 (1-100 available)
AT+FU3             // Set transmit power (FU1=lowest, FU3=highest)
AT+RX              // Query all settings
```

**Quick Setup via Arduino Serial Monitor:**
```cpp
void setup() {
    Serial.begin(9600);
    HC12.begin(9600);
    
    // Enter AT mode (if SET pin on GPIO, set it HIGH)
    // digitalWrite(SET_PIN, HIGH);
    
    delay(100);
    
    // Send commands
    HC12.println("AT+B9600");  // Set baud
    delay(100);
    HC12.println("AT+C001");   // Set channel
    delay(100);
    HC12.println("AT+FU3");    // Max power
    delay(100);
    
    // Exit AT mode
    // digitalWrite(SET_PIN, LOW);
}
```

**After configuration, SET pin must be connected to GND for normal operation!**

## 🚀 Usage

### Step 1: Power On Transmitter (Toy Car)

1. Ensure HC-12 SET pin is connected to GND
2. Power on the toy car
3. LED should blink slowly (indicating transmission)
4. Check serial monitor (if USB connected): should see data packets

### Step 2: Start Receiver

**Option 1 (ESP32 Bridge):**
1. Connect receiver ESP32 to PC via USB
2. Note the COM port (e.g., COM5)

**Option 2 (Direct USB-Serial):**
1. Connect USB-Serial adapter to PC
2. Note the COM port

### Step 3: Run Python Plotter

```bash
cd "Lab2 - Client"
python imu_plotter.py
```

1. Select the receiver's COM port
2. Baud rate: **115200** (for ESP32 bridge) or **9600** (for direct HC-12)
3. Click "Connect"
4. You should see real-time IMU data!

### Step 4: Move the Toy Car

- Accelerate → See ax change
- Brake → See braking event detected
- Turn → See gyroscope gz spike
- Hit a bump → See impact event detected

## 📊 Data Format

Transmitted every 20ms (50 Hz):

```
[CAMARADAS DO EDU]: ax, ay, az, gx, gy, gz\n
```

Example:
```
[CAMARADAS DO EDU]: 0.12, -0.05, 9.81, 1.23, -0.45, 0.78
```

Where:
- `ax, ay, az`: Acceleration in m/s² (X, Y, Z axes)
- `gx, gy, gz`: Angular velocity in deg/s (X, Y, Z axes)

## 🔧 Troubleshooting

### No data received

**Check HC-12 modules:**
- ✅ Both on same channel (AT+C001)
- ✅ Both at same baud rate (9600)
- ✅ SET pins connected to GND (not floating!)
- ✅ TX/RX not swapped
- ✅ Power LED on both modules

**Check wiring:**
- ✅ ESP32 TX2 → HC-12 RX
- ✅ ESP32 RX2 → HC-12 TX
- ✅ Common ground between ESP32 and HC-12

**Check power:**
- ✅ HC-12 needs 3.3V or 5V (check datasheet)
- ✅ Current draw can be 100mA+ during TX

### MPU6050 not found

**Check wiring:**
- ✅ SDA → GPIO 21
- ✅ SCL → GPIO 22
- ✅ VCC → 3.3V (NOT 5V!)
- ✅ GND → GND

**Check I2C:**
```cpp
// Add this to scan for I2C devices
Wire.beginTransmission(0x68);  // MPU6050 default address
if (Wire.endTransmission() == 0) {
    Serial.println("MPU6050 found at 0x68");
}
```

### Data corrupted or garbled

**Check baud rates match:**
- Transmitter HC-12: 9600
- Receiver HC-12: 9600
- Python plotter: 115200 (if using ESP32 bridge)

**Check for interference:**
- Keep HC-12 antennas vertical
- Separate from motors/ESCs (they generate RF noise)
- Add capacitors across motor terminals

### Range issues

**Improve range:**
- Use FU3 power mode: `AT+FU3`
- Use channel 001: `AT+C001` (better propagation)
- Keep antennas vertical and separated from metal
- Typical range: 100-600m in open air (varies by model)

## 🔋 Power Considerations

### Toy Car Power Budget

```
ESP32:      ~80-160mA (active with WiFi off)
MPU6050:    ~3.5mA
HC-12:      ~100mA (transmitting)
────────────────────────────
Total:      ~200mA average
```

**Recommendations:**
- Use 7.4V 2S LiPo battery (common in RC cars)
- Use 5V/3.3V voltage regulator
- Battery capacity: 1000mAh+ for 5+ hours runtime
- ESP32 deep sleep between transmissions (advanced optimization)

## 📈 Performance Optimization

### Increase Data Rate

```cpp
// In main.cpp, change:
#define SAMPLE_RATE_HZ  100  // Was 50
```

**Note:** HC-12 at 9600 baud can handle ~100 packets/sec max.

### Reduce Latency

```cpp
// Use faster HC-12 baud rate
AT+B115200  // Set HC-12 to 115200 baud

// Update code:
#define HC12_BAUD  115200
```

### Reduce Power

```cpp
// Use lower transmission power
AT+FU1  // Lowest power (shorter range, less current)

// Or use deep sleep between samples
esp_sleep_enable_timer_wakeup(20000);  // Wake every 20ms
esp_deep_sleep_start();
```

## 🎓 Understanding the Code

### Main Components

1. **MPU6050 Interface (I2C)**
   - Reads accelerometer (m/s²)
   - Reads gyroscope (rad/s → converted to deg/s)
   - Uses Adafruit library for easy access

2. **HC-12 Interface (UART2)**
   - Sends formatted strings
   - Simple print/println commands
   - No acknowledgment needed

3. **Timing Loop**
   - Fixed 50 Hz sampling using `millis()`
   - Non-blocking delays
   - Precise timing control

### Key Functions

```cpp
setupMPU6050()      // Initialize and configure IMU
setupHC12()         // Initialize UART for RF
transmitIMUData()   // Read sensors and transmit
blinkStatusLED()    // Visual feedback
```

## 🌟 Advanced Features (Future)

### Add GPS

```cpp
#include <TinyGPS++.h>
// Add GPS coordinates to transmitted data
```

### Add Battery Monitoring

```cpp
float batteryVoltage = analogRead(34) * (3.3/4095.0) * 2;
// Transmit battery level
```

### Two-Way Communication

```cpp
// Receive commands from base station
if (HC12.available()) {
    String command = HC12.readStringUntil('\n');
    // Control toy car based on commands
}
```

### Data Logging (SD Card)

```cpp
#include <SD.h>
// Log all IMU data to SD card for later analysis
```

## 📚 References

- [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [HC-12 User Manual](http://www.hc01.com/downloads)
- [PlatformIO Documentation](https://docs.platformio.org/)

## 📄 License

Created for educational purposes - Embedded Systems Lab

## 🤝 Support

For issues or questions:
1. Check troubleshooting section above
2. Review serial monitor output for error messages
3. Test each component separately
4. Check wiring with multimeter

Happy building! 🚗💨
