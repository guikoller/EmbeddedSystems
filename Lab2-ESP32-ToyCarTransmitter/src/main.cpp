/**
 * ESP32 Simple Serial Bridge
 * HC-12 (9600) -> USB Serial (115200)
 */

#include <Arduino.h>

#define HC12_TX_PIN     17
#define HC12_RX_PIN     16
#define HC12_BAUD       9600

HardwareSerial HC12(2);

void setup() {
    Serial.begin(115200);
    HC12.begin(HC12_BAUD, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
}

void loop() {
    if (HC12.available()) {
        Serial.write(HC12.read());
    }
}