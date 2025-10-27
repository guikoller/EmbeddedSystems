"""
IMU Data Simulator - Virtual Serial Port Data Generator
Author: AI Assistant
Date: October 27, 2025

This application simulates IMU sensor data and sends it via a virtual serial port.
Mouse movements control the simulated accelerometer and gyroscope values.
"""

import sys
import time
import math
import numpy as np
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QComboBox, QLineEdit, QLabel, QTextEdit, QGroupBox,
    QSlider, QCheckBox
)
from PySide6.QtCore import QThread, Signal, QTimer, Qt, QPoint
from PySide6.QtGui import QMouseEvent, QPalette, QColor
import serial
import serial.tools.list_ports

# ============================================================================
# CONFIGURATION
# ============================================================================

DATA_HEADER = "[CAMARADAS DO EDU]:"
UPDATE_RATE_HZ = 10  # Send data 50 times per second (20ms intervals - controlled rate)
GRAVITY = 9.81  # m/s²

# ============================================================================
# DATA GENERATOR THREAD
# ============================================================================

class IMUSimulatorThread(QThread):
    """
    Thread that generates and sends IMU data through serial port.
    """
    
    status_update = Signal(str)
    error_occurred = Signal(str)
    
    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.serial_connection = None
        self.running = False
        
        # Simulated sensor values
        self.ax = 0.0
        self.ay = 0.0
        self.az = GRAVITY
        self.gx = 0.0
        self.gy = 0.0
        self.gz = 0.0
        
        # Noise and drift parameters
        self.noise_enabled = True
        self.noise_level = 0.1
        
        # Special event simulation
        self.trigger_impact = False
        self.trigger_braking = False
        self.trigger_turn = False
        
        self.packets_sent = 0
    
    def run(self):
        """Main thread execution loop."""
        try:
            self.serial_connection = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1.0
            )
            self.running = True
            self.status_update.emit(f"Connected to {self.port}")
            
            interval = 1.0 / UPDATE_RATE_HZ
            
            while self.running:
                start_time = time.time()
                
                # Add noise to make it more realistic
                ax = self.ax
                ay = self.ay
                az = self.az
                gx = self.gx
                gy = self.gy
                gz = self.gz
                
                if self.noise_enabled:
                    ax += np.random.normal(0, self.noise_level)
                    ay += np.random.normal(0, self.noise_level)
                    az += np.random.normal(0, self.noise_level)
                    gx += np.random.normal(0, self.noise_level * 5)
                    gy += np.random.normal(0, self.noise_level * 5)
                    gz += np.random.normal(0, self.noise_level * 5)
                
                # Handle special event triggers
                if self.trigger_impact:
                    # Simulate a sudden impact (high jerk)
                    ax += np.random.uniform(-50, 50)
                    ay += np.random.uniform(-50, 50)
                    az += np.random.uniform(-50, 50)
                    self.trigger_impact = False
                
                if self.trigger_braking:
                    # Simulate braking (negative forward acceleration)
                    ax = -8.0 + np.random.normal(0, 0.5)
                
                if self.trigger_turn:
                    # Simulate sharp turn (high yaw rate)
                    gz = 120.0 if np.random.random() > 0.5 else -120.0
                
                # Format and send data
                data_string = f"{DATA_HEADER} {ax:.2f}, {ay:.2f}, {az:.2f}, {gx:.2f}, {gy:.2f}, {gz:.2f}\n"
                
                try:
                    self.serial_connection.write(data_string.encode('utf-8'))
                    self.packets_sent += 1
                    
                    if self.packets_sent % 50 == 0:
                        self.status_update.emit(f"Packets sent: {self.packets_sent}")
                
                except serial.SerialException as e:
                    self.error_occurred.emit(f"Serial error: {str(e)}")
                    self.running = False
                    break
                
                # Maintain update rate
                elapsed = time.time() - start_time
                sleep_time = max(0, interval - elapsed)
                time.sleep(sleep_time)
        
        except serial.SerialException as e:
            self.error_occurred.emit(f"Could not open port: {str(e)}")
        
        finally:
            if self.serial_connection and self.serial_connection.is_open:
                self.serial_connection.close()
                self.status_update.emit("Disconnected")
    
    def stop(self):
        """Stop the thread gracefully."""
        self.running = False
    
    def update_acceleration(self, ax, ay, az):
        """Update accelerometer values."""
        self.ax = ax
        self.ay = ay
        self.az = az
    
    def update_gyroscope(self, gx, gy, gz):
        """Update gyroscope values."""
        self.gx = gx
        self.gy = gy
        self.gz = gz
    
    def set_noise(self, enabled, level=0.1):
        """Set noise parameters."""
        self.noise_enabled = enabled
        self.noise_level = level


# ============================================================================
# INTERACTIVE CONTROL WIDGET
# ============================================================================

class MouseControlWidget(QWidget):
    """
    Widget that captures mouse movements and converts them to IMU data.
    """
    
    acceleration_changed = Signal(float, float, float)
    gyroscope_changed = Signal(float, float, float)
    
    def __init__(self):
        super().__init__()
        self.setMinimumSize(600, 400)
        self.setMouseTracking(True)
        
        # Set background color
        palette = self.palette()
        palette.setColor(QPalette.Window, QColor(240, 240, 240))
        self.setPalette(palette)
        self.setAutoFillBackground(True)
        
        # Mouse state
        self.mouse_pos = QPoint(300, 200)
        self.center_pos = QPoint(300, 200)
        self.mouse_pressed = False
        
        # Sensitivity factors
        self.accel_sensitivity = 20.0  # m/s² per 100 pixels
        self.gyro_sensitivity = 180.0  # deg/s per 100 pixels
        
        # Create info label
        self.info_label = QLabel(self)
        self.info_label.setGeometry(10, 10, 580, 100)
        self.info_label.setStyleSheet("""
            QLabel {
                background-color: rgba(255, 255, 255, 200);
                border: 2px solid #333;
                border-radius: 5px;
                padding: 10px;
                font-family: 'Courier New';
                font-size: 11pt;
            }
        """)
        self.info_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.update_info_label()
        
        # Timer for continuous updates
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.emit_values)
        self.update_timer.start(20)  # Update 50 times per second
    
    def resizeEvent(self, event):
        """Update center position when widget is resized."""
        self.center_pos = QPoint(self.width() // 2, self.height() // 2)
        self.info_label.setGeometry(10, 10, self.width() - 20, 100)
    
    def mouseMoveEvent(self, event: QMouseEvent):
        """Handle mouse movement."""
        self.mouse_pos = event.pos()
        self.update()
    
    def mousePressEvent(self, event: QMouseEvent):
        """Handle mouse press."""
        if event.button() == Qt.LeftButton:
            self.mouse_pressed = True
    
    def mouseReleaseEvent(self, event: QMouseEvent):
        """Handle mouse release."""
        if event.button() == Qt.LeftButton:
            self.mouse_pressed = False
    
    def emit_values(self):
        """Calculate and emit IMU values based on mouse position."""
        # Calculate offset from center
        dx = self.mouse_pos.x() - self.center_pos.x()
        dy = self.mouse_pos.y() - self.center_pos.y()
        
        # Convert to IMU values
        # Accelerometer: Mouse position controls tilt (affects acceleration)
        # X-axis: left-right movement
        # Y-axis: up-down movement (inverted because screen Y is inverted)
        # Z-axis: always close to gravity
        
        ax = (dx / 100.0) * self.accel_sensitivity
        ay = (-dy / 100.0) * self.accel_sensitivity  # Inverted Y
        az = GRAVITY - abs(dy / 200.0)  # Reduce Z component when tilted
        
        # Gyroscope: Mouse velocity controls rotation rate
        # For simplicity, use position as a proxy for rotation
        gx = (-dy / 100.0) * self.gyro_sensitivity  # Pitch (rotation around X)
        gy = (dx / 100.0) * self.gyro_sensitivity   # Roll (rotation around Y)
        gz = 0.0  # Yaw (controlled separately by horizontal movement speed)
        
        # If mouse is pressed, add yaw based on horizontal position
        if self.mouse_pressed:
            gz = (dx / 100.0) * self.gyro_sensitivity
        
        self.acceleration_changed.emit(ax, ay, az)
        self.gyroscope_changed.emit(gx, gy, gz)
        
        self.update_info_label()
    
    def update_info_label(self):
        """Update the information label with current values."""
        dx = self.mouse_pos.x() - self.center_pos.x()
        dy = self.mouse_pos.y() - self.center_pos.y()
        
        ax = (dx / 100.0) * self.accel_sensitivity
        ay = (-dy / 100.0) * self.accel_sensitivity
        az = GRAVITY - abs(dy / 200.0)
        
        gx = (-dy / 100.0) * self.gyro_sensitivity
        gy = (dx / 100.0) * self.gyro_sensitivity
        gz = (dx / 100.0) * self.gyro_sensitivity if self.mouse_pressed else 0.0
        
        info_text = f"""<b>Mouse Control Active</b><br>
<span style='color: blue;'>Move mouse to control acceleration & gyroscope</span><br>
<span style='color: green;'>Click & hold to activate yaw (gz)</span><br>
Accel: ({ax:6.2f}, {ay:6.2f}, {az:6.2f}) m/s²<br>
Gyro:  ({gx:6.2f}, {gy:6.2f}, {gz:6.2f}) deg/s"""
        
        self.info_label.setText(info_text)
    
    def paintEvent(self, event):
        """Draw visual feedback."""
        super().paintEvent(event)
        from PySide6.QtGui import QPainter, QPen
        
        painter = QPainter(self)
        
        # Draw center crosshair
        pen = QPen(QColor(100, 100, 100))
        pen.setWidth(2)
        painter.setPen(pen)
        
        center_x = self.center_pos.x()
        center_y = self.center_pos.y()
        
        painter.drawLine(center_x - 20, center_y, center_x + 20, center_y)
        painter.drawLine(center_x, center_y - 20, center_x, center_y + 20)
        
        # Draw mouse position indicator
        pen.setColor(QColor(255, 0, 0) if self.mouse_pressed else QColor(0, 0, 255))
        pen.setWidth(3)
        painter.setPen(pen)
        
        painter.drawEllipse(self.mouse_pos, 8, 8)
        
        # Draw line from center to mouse
        pen.setWidth(1)
        painter.setPen(pen)
        painter.drawLine(self.center_pos, self.mouse_pos)


# ============================================================================
# MAIN APPLICATION WINDOW
# ============================================================================

class IMUSimulatorApp(QMainWindow):
    """Main application window for the IMU Simulator."""
    
    def __init__(self):
        super().__init__()
        
        self.simulator_thread = None
        
        self.init_ui()
        self.refresh_ports()
    
    def init_ui(self):
        """Initialize the user interface."""
        self.setWindowTitle("IMU Data Simulator - Virtual Serial Port")
        self.setGeometry(100, 100, 800, 800)
        
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # ===== CONNECTION PANEL =====
        connection_group = QGroupBox("Virtual Serial Connection")
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
        
        self.connect_btn = QPushButton("Start Sending")
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.connect_btn.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_btn)
        
        connection_layout.addStretch()
        connection_group.setLayout(connection_layout)
        main_layout.addWidget(connection_group)
        
        # ===== CONTROL OPTIONS =====
        options_group = QGroupBox("Simulation Options")
        options_layout = QHBoxLayout()
        
        self.noise_checkbox = QCheckBox("Enable Noise")
        self.noise_checkbox.setChecked(True)
        self.noise_checkbox.stateChanged.connect(self.update_noise_settings)
        options_layout.addWidget(self.noise_checkbox)
        
        options_layout.addWidget(QLabel("Noise Level:"))
        self.noise_slider = QSlider(Qt.Horizontal)
        self.noise_slider.setMinimum(0)
        self.noise_slider.setMaximum(100)
        self.noise_slider.setValue(10)
        self.noise_slider.setMaximumWidth(150)
        self.noise_slider.valueChanged.connect(self.update_noise_settings)
        options_layout.addWidget(self.noise_slider)
        
        self.noise_label = QLabel("0.10")
        options_layout.addWidget(self.noise_label)
        
        options_layout.addStretch()
        options_group.setLayout(options_layout)
        main_layout.addWidget(options_group)
        
        # ===== EVENT TRIGGERS =====
        events_group = QGroupBox("Event Triggers (Click to Simulate)")
        events_layout = QHBoxLayout()
        
        impact_btn = QPushButton("⚠️ Trigger IMPACT")
        impact_btn.setStyleSheet("background-color: #FF5722; color: white; font-weight: bold; padding: 10px;")
        impact_btn.clicked.connect(self.trigger_impact)
        events_layout.addWidget(impact_btn)
        
        braking_btn = QPushButton("🛑 Trigger BRAKING")
        braking_btn.setStyleSheet("background-color: #FFC107; color: black; font-weight: bold; padding: 10px;")
        braking_btn.clicked.connect(self.trigger_braking)
        events_layout.addWidget(braking_btn)
        
        turn_btn = QPushButton("🔄 Trigger SHARP TURN")
        turn_btn.setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 10px;")
        turn_btn.clicked.connect(self.trigger_turn)
        events_layout.addWidget(turn_btn)
        
        events_group.setLayout(events_layout)
        main_layout.addWidget(events_group)
        
        # ===== MOUSE CONTROL AREA =====
        control_group = QGroupBox("Interactive Mouse Control")
        control_layout = QVBoxLayout()
        
        self.mouse_control = MouseControlWidget()
        self.mouse_control.acceleration_changed.connect(self.update_acceleration)
        self.mouse_control.gyroscope_changed.connect(self.update_gyroscope)
        control_layout.addWidget(self.mouse_control)
        
        control_group.setLayout(control_layout)
        main_layout.addWidget(control_group, stretch=1)
        
        # ===== STATUS PANEL =====
        status_group = QGroupBox("Status")
        status_layout = QVBoxLayout()
        
        self.status_display = QTextEdit()
        self.status_display.setReadOnly(True)
        self.status_display.setMaximumHeight(100)
        status_layout.addWidget(self.status_display)
        
        status_group.setLayout(status_layout)
        main_layout.addWidget(status_group)
        
        self.log_status("Simulator ready. Select a port and click 'Start Sending'.")
    
    def refresh_ports(self):
        """Refresh the list of available serial ports."""
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}", port.device)
        
        if self.port_combo.count() == 0:
            self.port_combo.addItem("No ports available", None)
    
    def toggle_connection(self):
        """Start or stop sending data."""
        if self.simulator_thread is None or not self.simulator_thread.isRunning():
            # Start sending
            port = self.port_combo.currentData()
            if port is None:
                self.log_status("ERROR: No valid port selected.", error=True)
                return
            
            try:
                baudrate = int(self.baudrate_input.text())
            except ValueError:
                self.log_status("ERROR: Invalid baud rate.", error=True)
                return
            
            self.simulator_thread = IMUSimulatorThread(port, baudrate)
            self.simulator_thread.status_update.connect(self.log_status)
            self.simulator_thread.error_occurred.connect(lambda msg: self.log_status(msg, error=True))
            self.simulator_thread.start()
            
            self.connect_btn.setText("Stop Sending")
            self.connect_btn.setStyleSheet("background-color: #f44336; color: white; font-weight: bold;")
            self.port_combo.setEnabled(False)
            self.baudrate_input.setEnabled(False)
        else:
            # Stop sending
            self.disconnect_simulator()
    
    def disconnect_simulator(self):
        """Stop the simulator."""
        if self.simulator_thread:
            self.simulator_thread.stop()
            self.simulator_thread.wait()
            self.simulator_thread = None
        
        self.connect_btn.setText("Start Sending")
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.port_combo.setEnabled(True)
        self.baudrate_input.setEnabled(True)
        
        self.log_status("Stopped sending data.")
    
    def update_acceleration(self, ax, ay, az):
        """Update acceleration values in the simulator."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            self.simulator_thread.update_acceleration(ax, ay, az)
    
    def update_gyroscope(self, gx, gy, gz):
        """Update gyroscope values in the simulator."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            self.simulator_thread.update_gyroscope(gx, gy, gz)
    
    def update_noise_settings(self):
        """Update noise settings."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            enabled = self.noise_checkbox.isChecked()
            level = self.noise_slider.value() / 100.0
            self.noise_label.setText(f"{level:.2f}")
            self.simulator_thread.set_noise(enabled, level)
    
    def trigger_impact(self):
        """Trigger an impact event."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            self.simulator_thread.trigger_impact = True
            self.log_status("⚠️ Impact event triggered!")
    
    def trigger_braking(self):
        """Trigger a braking event."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            self.simulator_thread.trigger_braking = not self.simulator_thread.trigger_braking
            if self.simulator_thread.trigger_braking:
                self.log_status("🛑 Braking event ON (will continue until clicked again)")
            else:
                self.log_status("🛑 Braking event OFF")
    
    def trigger_turn(self):
        """Trigger a sharp turn event."""
        if self.simulator_thread and self.simulator_thread.isRunning():
            self.simulator_thread.trigger_turn = True
            self.log_status("🔄 Sharp turn event triggered!")
    
    def log_status(self, message, error=False):
        """Log a message to the status display."""
        timestamp = time.strftime("%H:%M:%S")
        
        if error:
            formatted_message = f"<span style='color: red;'>[{timestamp}] {message}</span>"
        else:
            formatted_message = f"[{timestamp}] {message}"
        
        self.status_display.append(formatted_message)
    
    def closeEvent(self, event):
        """Clean up when the application is closed."""
        if self.simulator_thread:
            self.simulator_thread.stop()
            self.simulator_thread.wait()
        event.accept()


# ============================================================================
# APPLICATION ENTRY POINT
# ============================================================================

def main():
    """Main entry point for the application."""
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    
    window = IMUSimulatorApp()
    window.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
