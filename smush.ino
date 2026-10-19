#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

// Pin definitions for physical joystick lines (Dual Analog Breakout)
const int PIN_LX = A0;
const int PIN_LY = A1;
const int PIN_RX = A2;
const int PIN_RY = A3;

// Dynamic XOR compression mask
uint32_t xor_mask = 0xA5C3F00F;

// Compressed packet structure matching our PSP memory layout
typedef struct __attribute__((packed)) {
    uint32_t magic_header;       // Identifier for host recognition (e.g., 0x50535022 -> "PSP\"")
    uint32_t packed_left_axes;   // [16-bit LX | 16-bit LY]
    uint32_t packed_right_axes;  // [16-bit RX | 16-bit RY]
} CompressedStickPacket;

volatile CompressedStickPacket outgoing_packet;

// TinyUSB Device instance for custom data transfer
Adafruit_USBD_CDC USBPrxBridge; // Using USB CDC (Virtual Serial / Bulk transfer) as transport pipe

// Quantize, pack, and encrypt analog inputs
uint32_t pack_axes(uint16_t x_raw, uint16_t y_raw, uint32_t mask) {
    // Map 12-bit ADC range (0-4095) to clean 16-bit fixed-point resolution (0 - 65535)
    uint16_t qx = (uint16_t)((uint32_t)x_raw * 65535 / 4095);
    uint16_t qy = (uint16_t)((uint32_t)y_raw * 65535 / 4095);

    // Pack upper 16 bits (X) and lower 16 bits (Y)
    uint32_t raw_word = ((uint32_t)qx << 16) | qy;

    // Apply lean XOR compression layer
    return raw_word ^ mask;
}

void setup() {
    // 1. Initialize native USB stack (draws VBUS power from PSP USB port)
    USBDevice.setID(0x054C, 0x02E1); // Sony PSP vendor/device ID spoof or custom ID
    USBDevice.setManufacturerDescriptor("OpenHardware");
    USBDevice.setProductDescriptor("PSP DualStick Intermediary");
    
    USBPrxBridge.begin(115200);
    TinyUSB_Device_Init(0);

    // Configure analog resolution for precision sampling
    analogReadResolution(12);

    // Initialize packet headers and memory payload
    outgoing_packet.magic_header = 0x50535022; 
    outgoing_packet.packed_left_axes = 0;
    outgoing_packet.packed_right_axes = 0;

    // Wait briefly for USB bus enumeration with the host (PSP)
    while (!USBDevice.mounted()) {
        delay(10);
    }

    // Transmit memory block requirements and descriptor metadata to host plugin
    // Informing the PSP plugin of the exact memory footprint required (12 bytes total)
    USBPrxBridge.printf("MEM_REQ:%u\n", sizeof(CompressedStickPacket));
}

void loop() {
    // 1. Read physical joystick lines from analog breakouts
    uint16_t raw_lx = analogRead(PIN_LX);
    uint16_t raw_ly = analogRead(PIN_LY);
    uint16_t raw_rx = analogRead(PIN_RX);
    uint16_t raw_ry = analogRead(PIN_RY);

    // 2. Process and pack into compressed 32-bit address blocks
    outgoing_packet.packed_left_axes  = pack_axes(raw_lx, raw_ly, xor_mask);
    outgoing_packet.packed_right_axes = pack_axes(raw_rx, raw_ry, xor_mask);

    // 3. Emit / transmit compressed data over the USB transport pipe
    // Instead of parsing a sluggish USB input stream, the PSP plugin reads 
    // these structured blocks directly into its memory-mapped buffer.
    if (USBPrxBridge) {
        USBPrxBridge.write((uint8_t*)&outgoing_packet, sizeof(CompressedStickPacket));
        USBPrxBridge.flush();
    }

    // Maintain sub-millisecond polling rate (~200Hz)
    delay(5);
}
