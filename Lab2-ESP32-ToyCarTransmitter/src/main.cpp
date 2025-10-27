/**
 * ESP32 Toy Car IMU Transmitter
 * 
 * Hardware Setup:
 * ===============
 * MPU6050 (I2C):
 *   - VCC  → 3.3V
 *   - GND  → GND
 *   - SCL  → GPIO 22 (default I2C)
 *   - SDA  → GPIO 21 (default I2C)
 * 
 * HC-12 RF Module (UART):
 *   - VCC  → 5V (or 3.3V depending on module)
 *   - GND  → GND
 *   - TX   → GPIO 16 (RX2)
 *   - RX   → GPIO 17 (TX2)
 *   - SET  → GND (normal operation) or GPIO for AT commands
 * 
 * Features:
 * =========
 * - Reads 6-axis IMU data (accelerometer + gyroscope) from MPU6050
 * - Transmits data via HC-12 RF module at 50 Hz
 * - Protocol: [CAMARADAS DO EDU]: ax, ay, az, gx, gy, gz\n
 * - Built-in LED indicates transmission status
 * - Serial debug output at 115200 baud
 * 
 * Author: AI Assistant
 * Date: October 27, 2025
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// HC-12 UART Configuration
#define HC12_TX_PIN     17    // ESP32 TX2 → HC-12 RX
#define HC12_RX_PIN     16    // ESP32 RX2 → HC-12 TX
#define HC12_BAUD       9600  // HC-12 default baud rate

// I2C Configuration (MPU6050)
#define I2C_SDA         21    // Default SDA
#define I2C_SCL         22    // Default SCL
#define I2C_FREQ        400000 // 400kHz fast mode

// Transmission Settings
#define SAMPLE_RATE_HZ  50    // Send data 50 times per second
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE_HZ)

// Protocol
#define DATA_HEADER     "[CAMARADAS DO EDU]:"

// LED Pin (built-in LED on most ESP32 boards)
#define LED_PIN         2

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

Adafruit_MPU6050 mpu;
HardwareSerial HC12(2);  // Use UART2 for HC-12

// Timing
unsigned long lastTransmissionTime = 0;
unsigned long packetCount = 0;

// Status LED blink
unsigned long lastLedToggle = 0;
bool ledState = false;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void setupMPU6050();
void setupHC12();
void transmitIMUData();
void blinkStatusLED();
void printDebugInfo();

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════════╗");
    Serial.println("║  ESP32 Toy Car IMU Transmitter                 ║");
    Serial.println("║  MPU6050 + HC-12 RF Module                     ║");
    Serial.println("╚════════════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    
    // Initialize MPU6050
    setupMPU6050();
    
    // Initialize HC-12
    setupHC12();
    
    Serial.println("\n✓ System initialized successfully!");
    Serial.println("Starting data transmission...\n");
    
    // Initial LED blink to show ready
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long currentTime = millis();
    
    // Transmit data at fixed rate
    if (currentTime - lastTransmissionTime >= SAMPLE_INTERVAL_MS) {
        lastTransmissionTime = currentTime;
        transmitIMUData();
        blinkStatusLED();
        
        // Print debug info every second
        if (packetCount % SAMPLE_RATE_HZ == 0) {
            printDebugInfo();
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(1);
}

// ============================================================================
// FUNCTION IMPLEMENTATIONS
// ============================================================================

/**
 * Initialize and configure MPU6050 sensor
 */
void setupMPU6050() {
    Serial.println("→ Initializing MPU6050...");
    
    if (!mpu.begin()) {
        Serial.println("✗ Failed to find MPU6050 chip!");
        Serial.println("  Check wiring:");
        Serial.println("    VCC → 3.3V");
        Serial.println("    GND → GND");
        Serial.println("    SCL → GPIO 22");
        Serial.println("    SDA → GPIO 21");
        
        // Blink LED rapidly to indicate error
        while (1) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(200);
        }
    }
    
    Serial.println("✓ MPU6050 found!");
    
    // Configure sensor ranges
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    Serial.print("  Accelerometer range: ±");
    switch (mpu.getAccelerometerRange()) {
        case MPU6050_RANGE_2_G:  Serial.println("2G");  break;
        case MPU6050_RANGE_4_G:  Serial.println("4G");  break;
        case MPU6050_RANGE_8_G:  Serial.println("8G");  break;
        case MPU6050_RANGE_16_G: Serial.println("16G"); break;
    }
    
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    Serial.print("  Gyroscope range: ±");
    switch (mpu.getGyroRange()) {
        case MPU6050_RANGE_250_DEG:  Serial.println("250°/s");  break;
        case MPU6050_RANGE_500_DEG:  Serial.println("500°/s");  break;
        case MPU6050_RANGE_1000_DEG: Serial.println("1000°/s"); break;
        case MPU6050_RANGE_2000_DEG: Serial.println("2000°/s"); break;
    }
    
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.print("  Filter bandwidth: ");
    switch (mpu.getFilterBandwidth()) {
        case MPU6050_BAND_260_HZ: Serial.println("260 Hz"); break;
        case MPU6050_BAND_184_HZ: Serial.println("184 Hz"); break;
        case MPU6050_BAND_94_HZ:  Serial.println("94 Hz");  break;
        case MPU6050_BAND_44_HZ:  Serial.println("44 Hz");  break;
        case MPU6050_BAND_21_HZ:  Serial.println("21 Hz");  break;
        case MPU6050_BAND_10_HZ:  Serial.println("10 Hz");  break;
        case MPU6050_BAND_5_HZ:   Serial.println("5 Hz");   break;
    }
    
    // Let sensor stabilize
    Serial.println("  Calibrating sensor...");
    delay(100);
    
    // Discard first few readings
    for (int i = 0; i < 10; i++) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        delay(10);
    }
    
    Serial.println("✓ MPU6050 ready!");
}

/**
 * Initialize HC-12 RF module
 */
void setupHC12() {
    Serial.println("\n→ Initializing HC-12 RF Module...");
    
    // Initialize UART2 for HC-12
    HC12.begin(HC12_BAUD, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
    
    Serial.print("  TX Pin: GPIO ");
    Serial.println(HC12_TX_PIN);
    Serial.print("  RX Pin: GPIO ");
    Serial.println(HC12_RX_PIN);
    Serial.print("  Baud Rate: ");
    Serial.println(HC12_BAUD);
    
    // Wait for HC-12 to be ready
    delay(100);
    
    // Test transmission
    HC12.println("HC-12 Test");
    
    Serial.println("✓ HC-12 ready!");
    Serial.println("  NOTE: Make sure HC-12 SET pin is connected to GND");
    Serial.println("        for normal operation mode.");
}

/**
 * Read IMU data and transmit via HC-12
 */
void transmitIMUData() {
    // Read sensor data
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    
    // Extract values
    float ax = accel.acceleration.x;
    float ay = accel.acceleration.y;
    float az = accel.acceleration.z;
    float gx = gyro.gyro.x * 57.2958;  // Convert rad/s to deg/s
    float gy = gyro.gyro.y * 57.2958;
    float gz = gyro.gyro.z * 57.2958;
    
    // Format data string
    char dataBuffer[128];
    snprintf(dataBuffer, sizeof(dataBuffer),
             "%s %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
             DATA_HEADER, ax, ay, az, gx, gy, gz);
    
    // Transmit via HC-12
    HC12.print(dataBuffer);
    
    // Also output to USB Serial for debugging
    Serial.print(dataBuffer);
    
    packetCount++;
}

/**
 * Blink status LED to show activity
 */
void blinkStatusLED() {
    unsigned long currentTime = millis();
    
    // Blink every 500ms
    if (currentTime - lastLedToggle >= 500) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        lastLedToggle = currentTime;
    }
}

/**
 * Print debug information
 */
void printDebugInfo() {
    Serial.println("─────────────────────────────────────────");
    Serial.print("Packets sent: ");
    Serial.print(packetCount);
    Serial.print(" | Rate: ");
    Serial.print(SAMPLE_RATE_HZ);
    Serial.println(" Hz");
    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.println("─────────────────────────────────────────");
}
