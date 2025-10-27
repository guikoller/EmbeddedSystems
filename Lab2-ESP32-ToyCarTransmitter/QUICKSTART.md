# Quick Start Guide - ESP32 + HC-12 Setup

## 🚀 5-Minute Setup

### What You Need

**Transmitter (Toy Car):**
- [ ] ESP32 board
- [ ] MPU6050 sensor
- [ ] HC-12 RF module
- [ ] 6 jumper wires

**Receiver:**
- [ ] HC-12 RF module
- [ ] USB-to-Serial adapter (or another ESP32)

### Step 1: Wire the Transmitter (5 minutes)

```
MPU6050 → ESP32
─────────────────
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22

HC-12 → ESP32
─────────────────
VCC → 5V (or 3.3V*)
GND → GND
TX  → GPIO 16
RX  → GPIO 17
SET → GND (IMPORTANT!)

*Check your HC-12 module voltage
```

### Step 2: Upload Code (2 minutes)

```bash
# Open VS Code
# Install PlatformIO extension
# Open folder: Lab2-ESP32-ToyCarTransmitter
# Connect ESP32 via USB
# Click Upload button (→)
```

### Step 3: Configure HC-12 Modules (One time, 3 minutes)

**Both transmitter and receiver HC-12 must match!**

1. Connect HC-12 SET pin to 3.3V (temporarily)
2. Open Serial Monitor at 9600 baud
3. Type these commands:

```
AT+B9600     (press Enter) → Should reply "OK"
AT+C001      (press Enter) → Should reply "OK"  
AT+FU3       (press Enter) → Should reply "OK"
```

4. Reconnect SET pin to GND
5. Power cycle the module

**Do this for BOTH HC-12 modules!**

### Step 4: Setup Receiver (2 minutes)

**Option A - ESP32 Bridge (Easy):**
1. Wire HC-12 to second ESP32 (same as transmitter)
2. Upload `receiver.cpp.example` code
3. Connect to PC via USB

**Option B - Direct (Even Easier):**
1. Wire HC-12 TX → USB-Serial RX
2. Wire HC-12 RX → USB-Serial TX
3. Wire power and GND
4. Connect to PC via USB

### Step 5: Test! (1 minute)

```bash
cd "Lab2 - Client"
python imu_plotter.py
```

1. Select COM port
2. Baud: 115200 (ESP32) or 9600 (direct)
3. Click Connect
4. Move the transmitter → See data!

## ✅ Checklist

- [ ] All wires connected correctly
- [ ] HC-12 SET pins connected to GND
- [ ] Both HC-12 on channel 001
- [ ] Both HC-12 at 9600 baud
- [ ] Transmitter LED blinking
- [ ] Receiver getting data
- [ ] Python plotter showing graphs

## 🔴 Red Flags (Check These First!)

❌ **SET pin floating** → HC-12 won't transmit
❌ **TX/RX swapped** → No data
❌ **Different channels** → Can't communicate
❌ **MPU6050 on 5V** → Will damage sensor!
❌ **No common ground** → Erratic behavior

## 🎯 Expected Behavior

✅ Transmitter LED blinks every 0.5 sec
✅ Serial monitor shows data at 50 Hz
✅ Receiver gets packets (no errors)
✅ Python plotter shows smooth graphs
✅ Moving transmitter changes values
✅ Events detected correctly

## 📞 Quick Troubleshooting

| Problem | Solution |
|---------|----------|
| No data | Check SET pin = GND |
| Garbled data | Match baud rates |
| MPU6050 error | Check wiring, use 3.3V |
| Short range | Use AT+FU3, vertical antenna |
| Slow/laggy | Reduce PLOT_UPDATE_FPS in Python |

## 🎓 Test Commands

**Test MPU6050:**
```cpp
// In Serial Monitor, you should see:
✓ MPU6050 found!
  Accelerometer range: ±8G
  Gyroscope range: ±500°/s
```

**Test HC-12:**
```
// With SET=3.3V, type in Serial Monitor:
AT        → Should reply: OK
AT+RX     → Shows all settings
```

**Test Data Flow:**
```
Transmitter Serial → Should see:
[CAMARADAS DO EDU]: 0.12, -0.05, 9.81, 1.23, -0.45, 0.78
[CAMARADAS DO EDU]: 0.14, -0.06, 9.80, 1.25, -0.44, 0.79
...

Receiver Serial → Should see same data
Python Plotter → Should show graphs
```

## 🚗 Final Test - Put on Toy Car!

1. Disconnect USB from transmitter ESP32
2. Connect to toy car battery (through voltage regulator if needed)
3. Secure MPU6050 to car body (use double-sided tape)
4. Position HC-12 antenna vertically
5. Keep wires away from motors
6. Drive the car!
7. Watch real-time data in plotter!

Enjoy your wireless IMU system! 🎉
