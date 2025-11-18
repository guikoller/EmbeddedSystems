"""
Real-Time IMU Serial Plotter and Event Detector

Reads 6-axis IMU data from a serial port, plots in real-time, and
detects events using the SAME logic/thresholds as the STM32 firmware.
"""

import sys
import time
from collections import deque

import numpy as np
import serial
import serial.tools.list_ports
from PySide6.QtCore import QThread, Signal, QTimer
from PySide6.QtGui import QFont
from PySide6.QtWidgets import (
    QApplication,
    QComboBox,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QPushButton,
    QTabWidget,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)
import pyqtgraph as pg

# ============================================================================
# CONFIGURATION CONSTANTS
# ============================================================================

# MCU-aligned thresholds (RAW MPU6050 COUNTS)
ACCEL_BRAKE_THRESHOLD_RAW = -8000.0       # r.ax < -8000  -> BRAKE
ACCEL_CRASH_THRESHOLD_RAW = 20000.0       # ax + |ay| + |az| > 20000 -> CRASH
GYRO_CURVE_THRESHOLD_RAW = 8000.0         # |gz| > 8000 -> CURVE

# Data buffer size for plotting
MAX_PLOT_POINTS = 300  # a few seconds, depends on telem rate

# Plot update rate (FPS) - separate from data rate
PLOT_UPDATE_FPS = 15

# Serial data header
DATA_HEADER = "[CAMARADAS DO EDU]:"


# ============================================================================
# SERIAL COMMUNICATION THREAD
# ============================================================================

class SerialReaderThread(QThread):
    """
    Worker thread for reading serial data without blocking the UI.
    Emits parsed raw IMU data or error messages.
    """

    # timestamp, ax, ay, az, gx, gy, gz  (all RAW counts)
    data_received = Signal(float, float, float, float, float, float, float)
    error_occurred = Signal(str)
    connection_lost = Signal()

    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.serial_connection = None
        self.running = False

    def run(self):
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
                        line = self.serial_connection.readline().decode(
                            "utf-8", errors="ignore"
                        ).strip()

                        if line.startswith(DATA_HEADER):
                            data_part = line[len(DATA_HEADER):].strip()
                            values = data_part.split(",")

                            if len(values) == 6:
                                try:
                                    ax = float(values[0].strip())
                                    ay = float(values[1].strip())
                                    az = float(values[2].strip())
                                    gx = float(values[3].strip())
                                    gy = float(values[4].strip())
                                    gz = float(values[5].strip())

                                    timestamp = time.time()
                                    self.data_received.emit(
                                        timestamp, ax, ay, az, gx, gy, gz
                                    )
                                except ValueError:
                                    self.error_occurred.emit(
                                        f"Invalid data format: {data_part}"
                                    )
                            else:
                                self.error_occurred.emit(
                                    f"Expected 6 values, got {len(values)}"
                                )
                        # other lines ignored

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
        self.running = False


# ============================================================================
# MAIN APPLICATION WINDOW
# ============================================================================

class IMUPlotterApp(QMainWindow):
    """Main application window for the IMU Serial Plotter."""

    def __init__(self):
        super().__init__()

        # Data storage (RAW counts)
        self.time_data = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_x = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_y = deque(maxlen=MAX_PLOT_POINTS)
        self.accel_z = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_x = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_y = deque(maxlen=MAX_PLOT_POINTS)
        self.gyro_z = deque(maxlen=MAX_PLOT_POINTS)

        # Jerk (derivative of RAW accel) just for visualization
        self.jerk_x = deque(maxlen=MAX_PLOT_POINTS)
        self.jerk_y = deque(maxlen=MAX_PLOT_POINTS)
        self.jerk_z = deque(maxlen=MAX_PLOT_POINTS)

        # Previous values for jerk calculation (RAW)
        self.prev_ax = None
        self.prev_ay = None
        self.prev_az = None
        self.prev_timestamp = None

        # MCU-like event state (same as tDetect flags)
        self.brake_active = False
        self.crash_active = False
        self.curve_active = False

        # Serial thread
        self.serial_thread = None

        # Plot update control
        self.plot_needs_update = False
        self.plot_update_timer = QTimer()
        self.plot_update_timer.timeout.connect(self.update_plots_if_needed)
        self.plot_update_timer.start(int(1000 / PLOT_UPDATE_FPS))

        # Data rate monitoring
        self.packet_count = 0
        self.last_packet_time = time.time()
        self.current_data_rate = 0

        # Reference time for plots
        self.reference_time = None

        # UI Setup
        self.init_ui()

        # Auto-detect serial ports
        self.refresh_ports()

    def init_ui(self):
        self.setWindowTitle("Real-Time IMU Serial Plotter and Event Detector")
        self.setGeometry(100, 100, 1400, 900)

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
        self.connect_btn.setStyleSheet(
            "background-color: #4CAF50; color: white; font-weight: bold;"
        )
        self.connect_btn.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_btn)

        self.perf_label = QLabel("Data: -- pps | Display: 15 FPS")
        self.perf_label.setStyleSheet("color: #666; font-size: 10pt;")
        connection_layout.addWidget(self.perf_label)

        self.perf_update_timer = QTimer()
        self.perf_update_timer.timeout.connect(self.update_performance_label)
        self.perf_update_timer.start(1000)

        connection_layout.addStretch()
        connection_group.setLayout(connection_layout)
        main_layout.addWidget(connection_group)

        # ===== MIDDLE: PLOTTING AREA =====
        plot_tabs = QTabWidget()

        pg.setConfigOptions(antialias=True)

        # Accelerometer (RAW)
        self.accel_plot_widget = pg.PlotWidget(title="Accelerometer (RAW counts)")
        self.accel_plot_widget.setLabel("left", "Accel (raw)")
        self.accel_plot_widget.setLabel("bottom", "Time", units="s")
        self.accel_plot_widget.addLegend()
        self.accel_plot_widget.showGrid(x=True, y=True, alpha=0.3)

        self.accel_x_curve = self.accel_plot_widget.plot(
            pen=pg.mkPen(color="r", width=2), name="X"
        )
        self.accel_y_curve = self.accel_plot_widget.plot(
            pen=pg.mkPen(color="g", width=2), name="Y"
        )
        self.accel_z_curve = self.accel_plot_widget.plot(
            pen=pg.mkPen(color="b", width=2), name="Z"
        )

        plot_tabs.addTab(self.accel_plot_widget, "Accelerometer")

        # Gyroscope (RAW)
        self.gyro_plot_widget = pg.PlotWidget(title="Gyroscope (RAW counts)")
        self.gyro_plot_widget.setLabel("left", "Gyro (raw)")
        self.gyro_plot_widget.setLabel("bottom", "Time", units="s")
        self.gyro_plot_widget.addLegend()
        self.gyro_plot_widget.showGrid(x=True, y=True, alpha=0.3)

        self.gyro_x_curve = self.gyro_plot_widget.plot(
            pen=pg.mkPen(color="r", width=2), name="X"
        )
        self.gyro_y_curve = self.gyro_plot_widget.plot(
            pen=pg.mkPen(color="g", width=2), name="Y"
        )
        self.gyro_z_curve = self.gyro_plot_widget.plot(
            pen=pg.mkPen(color="b", width=2), name="Z"
        )

        plot_tabs.addTab(self.gyro_plot_widget, "Gyroscope")

        # Jerk (on RAW, mainly for shape)
        self.jerk_plot_widget = pg.PlotWidget(title="Jerk (d/dt of RAW accel)")
        self.jerk_plot_widget.setLabel("left", "Jerk (raw/s)")
        self.jerk_plot_widget.setLabel("bottom", "Time", units="s")
        self.jerk_plot_widget.addLegend()
        self.jerk_plot_widget.showGrid(x=True, y=True, alpha=0.3)

        self.jerk_x_curve = self.jerk_plot_widget.plot(
            pen=pg.mkPen(color="r", width=2), name="X"
        )
        self.jerk_y_curve = self.jerk_plot_widget.plot(
            pen=pg.mkPen(color="g", width=2), name="Y"
        )
        self.jerk_z_curve = self.jerk_plot_widget.plot(
            pen=pg.mkPen(color="b", width=2), name="Z"
        )

        plot_tabs.addTab(self.jerk_plot_widget, "Jerk")

        main_layout.addWidget(plot_tabs, stretch=3)

        # ===== BOTTOM: EVENT STATUS PANEL =====
        event_group = QGroupBox("Event Detection Status (MCU-synced)")
        event_layout = QVBoxLayout()

        self.event_display = QTextEdit()
        self.event_display.setReadOnly(True)
        self.event_display.setMaximumHeight(150)
        self.event_display.setFont(QFont("Courier", 10))
        event_layout.addWidget(self.event_display)

        event_group.setLayout(event_layout)
        main_layout.addWidget(event_group, stretch=1)

        self.log_event("Application started. Please connect to a serial port.")

    def refresh_ports(self):
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}", port.device)

        if self.port_combo.count() == 0:
            self.port_combo.addItem("No ports available", None)

    def toggle_connection(self):
        if self.serial_thread is None or not self.serial_thread.isRunning():
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
            self.connect_btn.setStyleSheet(
                "background-color: #f44336; color: white; font-weight: bold;"
            )
            self.port_combo.setEnabled(False)
            self.baudrate_input.setEnabled(False)

            self.log_event(f"Connected to {port} at {baudrate} baud.")
        else:
            self.disconnect_serial()

    def disconnect_serial(self):
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.wait()
            self.serial_thread = None

        self.connect_btn.setText("Connect")
        self.connect_btn.setStyleSheet(
            "background-color: #4CAF50; color: white; font-weight: bold;"
        )
        self.port_combo.setEnabled(True)
        self.baudrate_input.setEnabled(True)

        self.log_event("Disconnected from serial port.")

    def handle_data(self, timestamp, ax, ay, az, gx, gy, gz):
        """Handle incoming IMU data from the serial thread (RAW counts)."""
        # data rate
        self.packet_count += 1
        current_time = time.time()
        if current_time - self.last_packet_time >= 1.0:
            self.current_data_rate = self.packet_count
            self.packet_count = 0
            self.last_packet_time = current_time

        # reference time for x-axis
        if self.reference_time is None:
            self.reference_time = timestamp
        relative_time = timestamp - self.reference_time
        self.time_data.append(relative_time)

        # store raw data
        self.accel_x.append(ax)
        self.accel_y.append(ay)
        self.accel_z.append(az)
        self.gyro_x.append(gx)
        self.gyro_y.append(gy)
        self.gyro_z.append(gz)

        # jerk from RAW accel, only for visualization
        if self.prev_timestamp is not None:
            dt = timestamp - self.prev_timestamp
            if dt > 0:
                jx = (ax - self.prev_ax) / dt
                jy = (ay - self.prev_ay) / dt
                jz = (az - self.prev_az) / dt
                self.jerk_x.append(jx)
                self.jerk_y.append(jy)
                self.jerk_z.append(jz)
            else:
                self.jerk_x.append(0.0)
                self.jerk_y.append(0.0)
                self.jerk_z.append(0.0)
        else:
            self.jerk_x.append(0.0)
            self.jerk_y.append(0.0)
            self.jerk_z.append(0.0)

        self.prev_ax = ax
        self.prev_ay = ay
        self.prev_az = az
        self.prev_timestamp = timestamp

        # MCU-synced event detection (same logic as tDetect)
        self.detect_events_raw(ax, ay, az, gx, gy, gz)

        self.plot_needs_update = True

    def detect_events_raw(self, ax, ay, az, gx, gy, gz):
        """
        Event detection using EXACT same logic as STM32 tDetect(),
        but run on host side for synchronized visualization.
        All args are RAW counts.
        """

        # BRAKE: r.ax < ACCEL_BRAKE_THRESHOLD_RAW
        if (ax < ACCEL_BRAKE_THRESHOLD_RAW) and not self.brake_active:
            self.brake_active = True
            self.log_event(f"BRAKE  (ax = {ax:.0f})", event=True)
        elif ax > ACCEL_BRAKE_THRESHOLD_RAW:
            self.brake_active = False

        # CRASH: total_acc = ax + |ay| + |az| > ACCEL_CRASH_THRESHOLD_RAW
        total_acc = ax + ( -ay if ay < 0 else ay ) + ( -az if az < 0 else az )
        if (total_acc > ACCEL_CRASH_THRESHOLD_RAW) and not self.crash_active:
            self.crash_active = True
            self.log_event(f"CRASH  (ax + |ay| + |az| = {total_acc:.0f})", event=True)
        elif total_acc < (ACCEL_CRASH_THRESHOLD_RAW / 2.0):
            self.crash_active = False

        # CURVE: |gz| > GYRO_CURVE_THRESHOLD_RAW
        if (gz > GYRO_CURVE_THRESHOLD_RAW or gz < -GYRO_CURVE_THRESHOLD_RAW) and not self.curve_active:
            self.curve_active = True
            self.log_event(f"CURVE  (gz = {gz:.0f})", event=True)
        elif (-GYRO_CURVE_THRESHOLD_RAW / 2.0) < gz < (GYRO_CURVE_THRESHOLD_RAW / 2.0):
            self.curve_active = False

    def update_plots_if_needed(self):
        """Update plots only if new data has arrived (called by timer)."""
        if self.plot_needs_update and len(self.time_data) > 0:
            t = np.array(self.time_data)

            self.accel_x_curve.setData(t, np.array(self.accel_x))
            self.accel_y_curve.setData(t, np.array(self.accel_y))
            self.accel_z_curve.setData(t, np.array(self.accel_z))

            self.gyro_x_curve.setData(t, np.array(self.gyro_x))
            self.gyro_y_curve.setData(t, np.array(self.gyro_y))
            self.gyro_z_curve.setData(t, np.array(self.gyro_z))

            self.jerk_x_curve.setData(t, np.array(self.jerk_x))
            self.jerk_y_curve.setData(t, np.array(self.jerk_y))
            self.jerk_z_curve.setData(t, np.array(self.jerk_z))

            self.plot_needs_update = False

    def update_performance_label(self):
        data_rate_text = (
            f"{self.current_data_rate} pps"
            if self.current_data_rate > 0 else "-- pps"
        )
        self.perf_label.setText(
            f"Data: {data_rate_text} | Display: {PLOT_UPDATE_FPS} FPS | "
            f"Buffer: {len(self.time_data)}/{MAX_PLOT_POINTS}"
        )

    def log_event(self, message, error=False, event=False):
        ts = time.strftime("%H:%M:%S")
        if error:
            formatted = f"<span style='color: red;'>[{ts}] {message}</span>"
        elif event:
            formatted = (
                f"<span style='color: orange; font-weight: bold;'>"
                f"[{ts}] {message}</span>"
            )
        else:
            formatted = f"[{ts}] {message}"
        self.event_display.append(formatted)

    def handle_error(self, error_message):
        self.log_event(f"ERROR: {error_message}", error=True)

    def handle_disconnection(self):
        self.log_event("Connection lost. Please reconnect.", error=True)
        self.disconnect_serial()

    def closeEvent(self, event):
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.wait()
        event.accept()


# ============================================================================
# APPLICATION ENTRY POINT
# ============================================================================

def main():
    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    window = IMUPlotterApp()
    window.show()

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
