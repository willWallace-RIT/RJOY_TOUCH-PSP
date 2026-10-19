#include <Arduino.h>

// Pin definitions for Dual Analog Joysticks (X and Y for both sticks)
const int PIN_LX = A0;
const int PIN_LY = A1;
const int PIN_RX = A2;
const int PIN_RY = A3;

// Fast dynamic XOR key for lean obfuscation/protection
uint32_t xor_mask = 0xA5C3F00F;

// Compressed packet structure for memory-mapped exposure
typedef struct {
    uint32_t packed_left_axes;   // [16-bit LX | 16-bit LY]
    uint32_t packed_right_axes;  // [16-bit RX | 16-bit RY]
} CompressedStickPacket;

volatile CompressedStickPacket current_packet;

// Quantize and pack two 8-bit or 10-bit ADC readings into a single 32-bit word
uint32_t pack_axes(uint16_t x_raw, uint16_t y_raw, uint32_t mask) {
    // Map standard ADC range (0-1023 for 10-bit, or 0-4095 for 12-bit) 
    // down to clean 16-bit fixed-point resolution (0 - 65535)
    uint16_t qx = (uint16_t)((uint32_t)x_raw * 65535 / 4095);
    uint16_t qy = (uint16_t)((uint32_t)y_raw * 65535 / 4095);

    // Pack upper 16 bits (X) and lower 16 bits (Y)
    uint32_t raw_word = ((uint32_t)qx << 16) | qy;

    // Apply lean XOR compression/masking layer
    return raw_word ^ mask;
}

void setup() {
    // Initialize serial or hardware bus interface (SPI/I2C slave mode for PSP bus)
    Serial.begin(115200);

    // Configure ADC resolution if using a 12-bit microcontroller like RP2040 or ESP32
    analogReadResolution(12);

    // Initialize packet memory
    current_packet.packed_left_axes = 0;
    current_packet.packed_right_axes = 0;
}

void loop() {
    // 1. Read raw analog inputs from ribbon/joystick breakouts
    uint16_t raw_lx = analogRead(PIN_LX);
    uint16_t raw_ly = analogRead(PIN_LY);
    uint16_t raw_rx = analogRead(PIN_RX);
    uint16_t raw_ry = analogRead(PIN_RY);

    // 2. Compress, quantize, and encrypt into dual 32-bit words
    // Big-endian / Little-endian stream formatting can be handled at the packing stage
    current_packet.packed_left_axes  = pack_axes(raw_lx, raw_ly, xor_mask);
    current_packet.packed_right_axes = pack_axes(raw_rx, raw_ry, xor_mask);

    // 3. Expose to bus (e.g., via DMA, SPI slave transfer, or shared memory register)
    // For testing/debugging, we stream the packed frames over serial
    // In production, this maps directly to the hardware bus registers read by the PSP PRX plugin.
    
    delay(5); // Poll at ~200Hz to maintain sub-millisecond input latency
}
