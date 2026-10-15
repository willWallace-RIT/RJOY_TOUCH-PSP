#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t active;
} AnalogStick;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t touch_down;
    uint8_t pressure;
} TouchPoint;

typedef struct {
    AnalogStick secondary_stick;
    TouchPoint touch;
    uint32_t button_flags;
    uint64_t timestamp;
} UnifiedInputState;

// Standard Plugin Export Functions
EXPORT int plugin_init(const char* config_path);
EXPORT void plugin_poll(UnifiedInputState* out_state);
EXPORT uint32_t plugin_get_capabilities(void);
EXPORT void plugin_shutdown(void);
