# Real-Time IMU Serial Plotter and Event Detector

A Python desktop application that reads 6-axis IMU (accelerometer and gyroscope) data from a serial port, plots all 6 axes in real-time, calculates and plots derivatives (jerk), and detects specific motion events (impacts, braking, sharp turns).

## Features

### 📊 Real-Time Plotting
- **Accelerometer Plot**: Displays ax, ay, az in real-time with color-coded lines (X=Red, Y=Green, Z=Blue)
- **Gyroscope Plot**: Displays gx, gy, gz in real-time with color-coded lines
- **Jerk Plot**: Calculates and displays the derivative of acceleration (jerk) for all three axes

### 🎯 Event Detection
The application automatically detects and displays the following motion events:
- **⚠️ Impact Detection**: Triggered when jerk magnitude exceeds 100 m/s³
- **🛑 Braking Detection**: Triggered when ax < -5 m/s² for more than 0.25 seconds
- **🔄 Sharp Turn Detection**: Triggered when |gz| > 90 deg/s

### 🔌 Serial Communication
- Auto-detects available COM ports
- Configurable baud rate
- Robust error handling and parsing
- Dedicated thread for non-blocking serial communication

## Data Format

The application expects serial data in the following format:

```
[CAMARADAS DO EDU]: ax, ay, az, gx, gy, gz\n
```

**Example:**
```
[CAMARADAS DO EDU]: 1.25, -0.02, 9.81, 0.5, -0.1, 0.0
```

Where:
- `ax, ay, az`: Accelerometer values in m/s²
- `gx, gy, gz`: Gyroscope values in deg/s

Lines that don't start with the header `[CAMARADAS DO EDU]:` are automatically discarded.

## Installation

### Prerequisites
- Python 3.8 or higher
- pip (Python package installer)

### Setup Steps

1. **Clone or download this repository**

2. **Navigate to the project directory**
   ```powershell
   cd "c:\Users\g_kol\Documents\Repos\EmbeddedSystems\Lab2 - Client"
   ```

3. **Install required dependencies**
   ```powershell
   pip install -r requirements.txt
   ```

   This will install:
   - PySide6 (Qt framework for Python)
   - pyqtgraph (high-performance plotting)
   - pyserial (serial communication)
   - numpy (numerical operations)

## Usage

### Running the Application

```powershell
python imu_plotter.py
```

### Using the Application

1. **Connect to Serial Port**
   - Select your COM port from the dropdown menu
   - Click "Refresh" if you don't see your device
   - Enter the baud rate (default: 115200)
   - Click "Connect"

2. **View Real-Time Data**
   - Switch between tabs to view:
     - Accelerometer data
     - Gyroscope data
     - Jerk (acceleration derivative)
   - The plots auto-scroll and display the last 500 data points

3. **Monitor Events**
   - Watch the Event Detection Status panel at the bottom
   - Events are highlighted with timestamps and color coding:
     - Red: Errors
     - Orange: Detected events (Impact, Braking, Sharp Turn)

4. **Disconnect**
   - Click "Disconnect" to stop receiving data
   - The application can be safely closed at any time

## Configuration

You can easily adjust the event detection thresholds by modifying the constants at the top of `imu_plotter.py`:

```python
# Event Detection Thresholds (easily adjustable)
IMPACT_JERK_THRESHOLD = 100.0  # m/s³
BRAKING_ACCEL_THRESHOLD = -5.0  # m/s²
BRAKING_DURATION_MIN = 0.25  # seconds
SHARP_TURN_GYRO_THRESHOLD = 90.0  # deg/s

# Data buffer size for plotting
MAX_PLOT_POINTS = 500
```

## Troubleshooting

### No COM ports detected
- Ensure your device is properly connected
- Check that the device drivers are installed
- Click the "Refresh" button to rescan for ports

### Connection errors
- Verify the correct baud rate for your device
- Ensure no other application is using the COM port
- Check that your device is sending data in the correct format

### No data displayed
- Verify that your device is sending data with the correct header: `[CAMARADAS DO EDU]:`
- Check the Event Status panel for parsing errors
- Ensure data is formatted as comma-separated values

### Application freezes
- The serial communication runs in a separate thread to prevent UI freezing
- If issues persist, try disconnecting and reconnecting
- Check the Event Status panel for error messages

## Technical Details

### Architecture
- **UI Framework**: PySide6 (Qt for Python)
- **Plotting Engine**: pyqtgraph (optimized for real-time updates)
- **Threading**: Dedicated QThread for serial communication to prevent UI blocking
- **Signal/Slot Pattern**: Qt's signal/slot mechanism for thread-safe communication

### Event Detection Algorithms

**Impact Detection:**
- Calculates jerk magnitude: `sqrt(jx² + jy² + jz²)`
- Triggers when magnitude exceeds threshold

**Braking Detection:**
- Monitors ax (forward acceleration)
- Triggers when ax is negative and sustained for minimum duration

**Sharp Turn Detection:**
- Monitors gz (yaw angular velocity)
- Triggers when absolute value exceeds threshold

## License

This project was created for educational purposes as part of an Embedded Systems course.

## Author

Created on October 27, 2025

## Support

For issues or questions, please refer to the course materials or contact your instructor.
