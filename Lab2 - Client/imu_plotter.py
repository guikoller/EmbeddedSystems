"""
Real-Time IMU Serial Plotter and Event Detector
Author: AI Assistant
Date: October 27, 2025

This application reads 6-axis IMU data from a serial port, plots the data in real-time,
calculates derivatives (jerk), and detects motion events (impacts, braking, sharp turns).
"""

import sys
import numpy as np
from collections import deque
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QComboBox, QLineEdit, QLabel, QTextEdit, QTabWidget, QGroupBox
)
from PySide6.QtCore import QThread, Signal, QTimer, Qt
from PySide6.QtGui import QFont
import pyqtgraph as pg
import serial
import serial.tools.list_ports
import time

# ============================================================================
# CONFIGURATION CONSTANTS
# ============================================================================

# Event Detection Thresholds (easily adjustable)
IMPACT_JERK_THRESHOLD = 100.0  # m/s³
BRAKING_ACCEL_THRESHOLD = -5.0  # m/s²
BRAKING_DURATION_MIN = 0.25  # seconds
SHARP_TURN_GYRO_THRESHOLD = 90.0  # deg/s

# Data buffer size for plotting
MAX_PLOT_POINTS = 30  # ~2 seconds at 100Hz

# Plot update rate (FPS) - separate from data rate
PLOT_UPDATE_FPS = 15  # Update plots 15 times per second (smooth but not overwhelming)

# Serial data header
DATA_HEADER = "[CAMARADAS DO EDU]:"


# ============================================================================
# SERIAL COMMUNICATION THREAD
# ============================================================================

class SerialReaderThread(QThread):
    """
    Worker thread for reading serial data without blocking the UI.
    Emits parsed IMU data or error messages.
    """
    
    data_received = Signal(float, float, float, float, float, float, float)  # timestamp, ax, ay, az, gx, gy, gz
    error_occurred = Signal(str)
    connection_lost = Signal()
    
    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.serial_connection = None
        self.running = False
    
    def run(self):
        """Main thread execution loop."""
        try:
            self.serial_connection = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1.0
            )
            self.running = True
            
            while self.running:
                try:
                    if self.serial_connection.in_waiting > 0:
                        # Read a complete line (until \n)
                        line = self.serial_connection.readline().decode('utf-8', errors='ignore').strip()
                        
                        # Check if the line starts with the required header
                        if line.startswith(DATA_HEADER):
                            # Extract the data part (after the header)
                            data_part = line[len(DATA_HEADER):].strip()
                            
                            # Parse the comma-separated values
                            values = data_part.split(',')
                            
                            if len(values) == 6:
                                try:
                                    ax = float(values[0].strip())
                                    ay = float(values[1].strip())
                                    az = float(values[2].strip())
                                    gx = float(values[3].strip())
                                    gy = float(values[4].strip())
                                    gz = float(values[5].strip())
                                    
                                    timestamp = time.time()
                                    
                                    # Emit the parsed data
                                    self.data_received.emit(timestamp, ax, ay, az, gx, gy, gz)
                                    
                                except ValueError:
                                    self.error_occurred.emit(f"Invalid data format: {data_part}")
                            else:
                                self.error_occurred.emit(f"Expected 6 values, got {len(values)}")
                        # Lines without the header are silently discarded
                
                except serial.SerialException as e:
                    self.error_occurred.emit(f"Serial error: {str(e)}")
                    self.running = False
                    self.connection_lost.emit()
                    break
                except Exception as e:
                    self.error_occurred.emit(f"Unexpected error: {str(e)}")
        
        except serial.SerialException as e:
            self.error_occurred.emit(f"Could not open port: {str(e)}")
            self.connection_lost.emit()
        
        finally:
            if self.serial_connection and self.serial_connection.is_open:
                self.serial_connection.close()
    
    def stop(self):
        """Stop the thread gracefully."""
        self.running = False


# ============================================================================
# MAIN APPLICATION WINDOW
# ============================================================================

class IMUPlotterApp(QMainWindow):
    """Main application window for the IMU Serial Plotter."""
    
    def __init__(self):
        super().__init__()
        
        # Data storage
        self.time_data = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_x = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_y = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_z = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_x = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_y = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_z = deque(maxlen=MAX_PLOT_POINTS)
        self.jerk_x = deque(maxlen=MAX_PLOT_POINTS)
        self.jerk_y = deque(maxlen=MAX_PLOT_POINTS)
        self.jerk_z = deque(maxlen=MAX_PLOT_POINTS)
        
        # Previous values for jerk calculation
        self.prev_ax = None
        self.prev_ay = None
        self.prev_az = None
        self.prev_timestamp = None
        
        # Braking detection state
        self.braking_start_time = None
        
        # Serial thread
        self.serial_thread = None
        
        # Plot update control
        self.plot_needs_update = False
        self.plot_update_timer = QTimer()
        self.plot_update_timer.timeout.connect(self.update_plots_if_needed)
        self.plot_update_timer.start(int(1000 / PLOT_UPDATE_FPS))  # Update at fixed FPS
        
        # Data rate monitoring
        self.packet_count = 0
        self.last_packet_time = time.time()
        self.current_data_rate = 0
        
        # UI Setup
        self.init_ui()
        
        # Auto-detect available serial ports
        self.refresh_ports()
    
    def init_ui(self):
        """Initialize the user interface."""
        self.setWindowTitle("Real-Time IMU Serial Plotter and Event Detector")
        self.setGeometry(100, 100, 1400, 900)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # ===== TOP: CONNECTION PANEL =====
        connection_group = QGroupBox("Serial Connection")
        connection_layout = QHBoxLayout()
        
        connection_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
        connection_layout.addWidget(self.port_combo)
        
        refresh_btn = QPushButton("Refresh")
        refresh_btn.clicked.connect(self.refresh_ports)
        connection_layout.addWidget(refresh_btn)
        
        connection_layout.addWidget(QLabel("Baud Rate:"))
        self.baudrate_input = QLineEdit("115200")
        self.baudrate_input.setMaximumWidth(100)
        connection_layout.addWidget(self.baudrate_input)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.connect_btn.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_btn)
        
        # Performance status label
        self.perf_label = QLabel("Data: -- pps | Display: 30 FPS")
        self.perf_label.setStyleSheet("color: #666; font-size: 10pt;")
        connection_layout.addWidget(self.perf_label)
        
        # Timer to update performance stats
        self.perf_update_timer = QTimer()
        self.perf_update_timer.timeout.connect(self.update_performance_label)
        self.perf_update_timer.start(1000)  # Update every second
        
        connection_layout.addStretch()
        connection_group.setLayout(connection_layout)
        main_layout.addWidget(connection_group)
        
        # ===== MIDDLE: PLOTTING AREA =====
        plot_tabs = QTabWidget()
        
        # Configure pyqtgraph
        pg.setConfigOptions(antialias=True)
        
        # Plot 1: Accelerometer
        self.accel_plot_widget = pg.PlotWidget(title="Accelerometer Data")
        self.accel_plot_widget.setLabel('left', 'Acceleration', units='m/s²')
        self.accel_plot_widget.setLabel('bottom', 'Time', units='s')
        self.accel_plot_widget.addLegend()
        self.accel_plot_widget.showGrid(x=True, y=True, alpha=0.3)
        
        self.accel_x_curve = self.accel_plot_widget.plot(pen=pg.mkPen(color='r', width=2), name='X')
        self.accel_y_curve = self.accel_plot_widget.plot(pen=pg.mkPen(color='g', width=2), name='Y')
        self.accel_z_curve = self.accel_plot_widget.plot(pen=pg.mkPen(color='b', width=2), name='Z')
        
        plot_tabs.addTab(self.accel_plot_widget, "Accelerometer")
        
        # Plot 2: Gyroscope
        self.gyro_plot_widget = pg.PlotWidget(title="Gyroscope Data")
        self.gyro_plot_widget.setLabel('left', 'Angular Velocity', units='deg/s')
        self.gyro_plot_widget.setLabel('bottom', 'Time', units='s')
        self.gyro_plot_widget.addLegend()
        self.gyro_plot_widget.showGrid(x=True, y=True, alpha=0.3)
        
        self.gyro_x_curve = self.gyro_plot_widget.plot(pen=pg.mkPen(color='r', width=2), name='X')
        self.gyro_y_curve = self.gyro_plot_widget.plot(pen=pg.mkPen(color='g', width=2), name='Y')
        self.gyro_z_curve = self.gyro_plot_widget.plot(pen=pg.mkPen(color='b', width=2), name='Z')
        
        plot_tabs.addTab(self.gyro_plot_widget, "Gyroscope")
        
        # Plot 3: Jerk (Derivative of Acceleration)
        self.jerk_plot_widget = pg.PlotWidget(title="Jerk (Acceleration Derivative)")
        self.jerk_plot_widget.setLabel('left', 'Jerk', units='m/s³')
        self.jerk_plot_widget.setLabel('bottom', 'Time', units='s')
        self.jerk_plot_widget.addLegend()
        self.jerk_plot_widget.showGrid(x=True, y=True, alpha=0.3)
        
        self.jerk_x_curve = self.jerk_plot_widget.plot(pen=pg.mkPen(color='r', width=2), name='X')
        self.jerk_y_curve = self.jerk_plot_widget.plot(pen=pg.mkPen(color='g', width=2), name='Y')
        self.jerk_z_curve = self.jerk_plot_widget.plot(pen=pg.mkPen(color='b', width=2), name='Z')
        
        plot_tabs.addTab(self.jerk_plot_widget, "Jerk")
        
        main_layout.addWidget(plot_tabs, stretch=3)
        
        # ===== BOTTOM: EVENT STATUS PANEL =====
        event_group = QGroupBox("Event Detection Status")
        event_layout = QVBoxLayout()
        
        self.event_display = QTextEdit()
        self.event_display.setReadOnly(True)
        self.event_display.setMaximumHeight(150)
        self.event_display.setFont(QFont("Courier", 10))
        event_layout.addWidget(self.event_display)
        
        event_group.setLayout(event_layout)
        main_layout.addWidget(event_group, stretch=1)
        
        # Add initial message
        self.log_event("Application started. Please connect to a serial port.")
    
    def refresh_ports(self):
        """Refresh the list of available serial ports."""
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}", port.device)
        
        if self.port_combo.count() == 0:
            self.port_combo.addItem("No ports available", None)
    
    def toggle_connection(self):
        """Connect or disconnect from the serial port."""
        if self.serial_thread is None or not self.serial_thread.isRunning():
            # Connect
            port = self.port_combo.currentData()
            if port is None:
                self.log_event("ERROR: No valid port selected.", error=True)
                return
            
            try:
                baudrate = int(self.baudrate_input.text())
            except ValueError:
                self.log_event("ERROR: Invalid baud rate.", error=True)
                return
            
            self.serial_thread = SerialReaderThread(port, baudrate)
            self.serial_thread.data_received.connect(self.handle_data)
            self.serial_thread.error_occurred.connect(self.handle_error)
            self.serial_thread.connection_lost.connect(self.handle_disconnection)
            self.serial_thread.start()
            
            self.connect_btn.setText("Disconnect")
            self.connect_btn.setStyleSheet("background-color: #f44336; color: white; font-weight: bold;")
            self.port_combo.setEnabled(False)
            self.baudrate_input.setEnabled(False)
            
            self.log_event(f"Connected to {port} at {baudrate} baud.")
        else:
            # Disconnect
            self.disconnect_serial()
    
    def disconnect_serial(self):
        """Disconnect from the serial port."""
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.wait()
            self.serial_thread = None
        
        self.connect_btn.setText("Connect")
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.port_combo.setEnabled(True)
        self.baudrate_input.setEnabled(True)
        
        self.log_event("Disconnected from serial port.")
    
    def handle_data(self, timestamp, ax, ay, az, gx, gy, gz):
        """Handle incoming IMU data from the serial thread."""
        # Monitor data rate
        self.packet_count += 1
        current_time = time.time()
        if current_time - self.last_packet_time >= 1.0:
            # Calculate packets per second
            self.current_data_rate = self.packet_count
            self.packet_count = 0
            self.last_packet_time = current_time
        
        # Store accelerometer and gyroscope data
        if len(self.time_data) == 0:
            reference_time = timestamp
            self.reference_time = reference_time
        else:
            reference_time = self.reference_time
        
        relative_time = timestamp - reference_time
        self.time_data.append(relative_time)
        
        self.accel_x.append(ax)
        self.accel_y.append(ay)
        self.accel_z.append(az)
        self.gyro_x.append(gx)
        self.gyro_y.append(gy)
        self.gyro_z.append(gz)
        
        # Calculate jerk (derivative of acceleration)
        if self.prev_timestamp is not None:
            delta_time = timestamp - self.prev_timestamp
            if delta_time > 0:
                jx = (ax - self.prev_ax) / delta_time
                jy = (ay - self.prev_ay) / delta_time
                jz = (az - self.prev_az) / delta_time
                
                self.jerk_x.append(jx)
                self.jerk_y.append(jy)
                self.jerk_z.append(jz)
                
                # Event Detection
                self.detect_events(ax, ay, az, gx, gy, gz, jx, jy, jz, timestamp)
            else:
                self.jerk_x.append(0.0)
                self.jerk_y.append(0.0)
                self.jerk_z.append(0.0)
        else:
            self.jerk_x.append(0.0)
            self.jerk_y.append(0.0)
            self.jerk_z.append(0.0)
        
        # Update previous values
        self.prev_ax = ax
        self.prev_ay = ay
        self.prev_az = az
        self.prev_timestamp = timestamp
        
        # Mark that plot needs update (actual update happens in timer)
        self.plot_needs_update = True
    
    def detect_events(self, ax, ay, az, gx, gy, gz, jx, jy, jz, timestamp):
        """Detect motion events based on sensor data and derivatives."""
        
        # 1. IMPACT DETECTION
        # Calculate jerk magnitude
        jerk_magnitude = np.sqrt(jx**2 + jy**2 + jz**2)
        if jerk_magnitude > IMPACT_JERK_THRESHOLD:
            self.log_event(f"⚠️ IMPACT DETECTED! (Jerk magnitude: {jerk_magnitude:.2f} m/s³)", event=True)
        
        # 2. BRAKING DETECTION
        # Check if ax is below the braking threshold
        if ax < BRAKING_ACCEL_THRESHOLD:
            if self.braking_start_time is None:
                self.braking_start_time = timestamp
            else:
                braking_duration = timestamp - self.braking_start_time
                if braking_duration >= BRAKING_DURATION_MIN:
                    self.log_event(f"🛑 BRAKING (ax: {ax:.2f} m/s², duration: {braking_duration:.2f}s)", event=True)
                    # Reset to avoid repeated messages
                    self.braking_start_time = timestamp
        else:
            self.braking_start_time = None
        
        # 3. SHARP TURN DETECTION
        # Check if |gz| exceeds the threshold (assuming gz is the yaw axis)
        if abs(gz) > SHARP_TURN_GYRO_THRESHOLD:
            self.log_event(f"🔄 SHARP TURN (gz: {gz:.2f} deg/s)", event=True)
    
    def update_plots_if_needed(self):
        """Update plots only if new data has arrived (called by timer at fixed FPS)."""
        if self.plot_needs_update and len(self.time_data) > 0:
            time_array = np.array(self.time_data)
            
            # Update accelerometer plot
            self.accel_x_curve.setData(time_array, np.array(self.accel_x))
            self.accel_y_curve.setData(time_array, np.array(self.accel_y))
            self.accel_z_curve.setData(time_array, np.array(self.accel_z))
            
            # Update gyroscope plot
            self.gyro_x_curve.setData(time_array, np.array(self.gyro_x))
            self.gyro_y_curve.setData(time_array, np.array(self.gyro_y))
            self.gyro_z_curve.setData(time_array, np.array(self.gyro_z))
            
            # Update jerk plot
            self.jerk_x_curve.setData(time_array, np.array(self.jerk_x))
            self.jerk_y_curve.setData(time_array, np.array(self.jerk_y))
            self.jerk_z_curve.setData(time_array, np.array(self.jerk_z))
            
            self.plot_needs_update = False
    
    def update_plots(self):
        """Update all plot widgets with the latest data."""
        if len(self.time_data) > 0:
            time_array = np.array(self.time_data)
            
            # Update accelerometer plot
            self.accel_x_curve.setData(time_array, np.array(self.accel_x))
            self.accel_y_curve.setData(time_array, np.array(self.accel_y))
            self.accel_z_curve.setData(time_array, np.array(self.accel_z))
            
            # Update gyroscope plot
            self.gyro_x_curve.setData(time_array, np.array(self.gyro_x))
            self.gyro_y_curve.setData(time_array, np.array(self.gyro_y))
            self.gyro_z_curve.setData(time_array, np.array(self.gyro_z))
            
            # Update jerk plot
            self.jerk_x_curve.setData(time_array, np.array(self.jerk_x))
            self.jerk_y_curve.setData(time_array, np.array(self.jerk_y))
            self.jerk_z_curve.setData(time_array, np.array(self.jerk_z))
    
    def update_performance_label(self):
        """Update the performance status label."""
        data_rate_text = f"{self.current_data_rate} pps" if self.current_data_rate > 0 else "-- pps"
        self.perf_label.setText(f"Data: {data_rate_text} | Display: {PLOT_UPDATE_FPS} FPS | Buffer: {len(self.time_data)}/{MAX_PLOT_POINTS}")
    
    def log_event(self, message, error=False, event=False):
        """Log a message to the event status panel."""
        timestamp = time.strftime("%H:%M:%S")
        
        if error:
            formatted_message = f"<span style='color: red;'>[{timestamp}] {message}</span>"
        elif event:
            formatted_message = f"<span style='color: orange; font-weight: bold;'>[{timestamp}] {message}</span>"
        else:
            formatted_message = f"[{timestamp}] {message}"
        
        self.event_display.append(formatted_message)
    
    def handle_error(self, error_message):
        """Handle errors from the serial thread."""
        self.log_event(f"ERROR: {error_message}", error=True)
    
    def handle_disconnection(self):
        """Handle unexpected disconnection."""
        self.log_event("Connection lost. Please reconnect.", error=True)
        self.disconnect_serial()
    
    def closeEvent(self, event):
        """Clean up when the application is closed."""
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.wait()
        event.accept()


# ============================================================================
# APPLICATION ENTRY POINT
# ============================================================================

def main():
    """Main entry point for the application."""
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # Modern look
    
    window = IMUPlotterApp()
    window.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
