#include <pspkernel.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <stdint.h>

PSP_MODULE_INFO("DualStickPlugin", PSP_MODULE_USER, 1, 1);
PSP_HEAP_SIZE_KB(512);

// Define our custom memory-mapped peripheral address space pointer
// (Replace with your actual hardware bus address or shared memory block)
#define PERIPHERAL_BASE_ADDR 0x04000000 
#define REG_RIGHT_STICK_X    (*(volatile uint32_t *)(PERIPHERAL_BASE_ADDR + 0x00))
#define REG_RIGHT_STICK_Y    (*(volatile uint32_t *)(PERIPHERAL_BASE_ADDR + 0x04))

// Extended control structure containing native left stick + custom right stick floats
typedef struct {
    unsigned int timeStamp;
    unsigned int buttons;
    unsigned char Lx; 
    unsigned char Ly; 
    unsigned char rsrv[6];
    
    // Injected right stick floats (parsed from little-endian peripheral register)
    float right_rx;
    float right_ry;
} ExtendedCtrlData;

// Original function pointer for hooking
static int (*g_original_sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count) = NULL;

// Helper to handle endianness swapping if hardware streams mixed formats
static inline uint32_t swap_endian32(uint32_t val) {
    return __builtin_bswap32(val);
}

// Hooked replacement for sceCtrlReadBufferPositive
int hooked_sceCtrlReadBufferPositive(SceCtrlData *pad_data, int count) {
    // 1. Call the original function to get standard hardware pad data (Left Stick, Buttons)
    int res = g_original_sceCtrlReadBufferPositive(pad_data, count);
    
    if (res > 0 && pad_data != NULL) {
        // Read raw register values from the peripheral address space
        uint32_t raw_rx_bytes = REG_RIGHT_STICK_X;
        uint32_t raw_ry_bytes = REG_RIGHT_STICK_Y;

        // Ensure proper little-endian float reconstruction from peripheral stream
        // If hardware streams big-endian, swap it first; if little-endian, process directly.
        uint32_t clean_rx = swap_endian32(raw_rx_bytes);
        uint32_t clean_ry = swap_endian32(raw_ry_bytes);

        // Map raw bits into IEEE 754 floats
        float rx_float = *((float *)&clean_rx);
        float ry_float = *((float *)&clean_ry);

        // If the target game expects the extended structure, we cast the buffer
        // or populate a side-channel shared memory block. For games reading standard 
        // SceCtrlData, we can store the secondary data in reserved bytes or a secondary buffer.
        ExtendedCtrlData *ext_data = (ExtendedCtrlData *)pad_data;
        for (int i = 0; i < res; i++) {
            ext_data[i].right_rx = rx_float;
            ext_data[i].right_ry = ry_float;
        }
    }
    
    return res;
}
