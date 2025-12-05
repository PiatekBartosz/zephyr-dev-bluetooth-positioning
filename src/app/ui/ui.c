// #include <zephyr/drivers/gpio.h>
// #include <zephyr/kernel.h>
// #include <zephyr/logging/log.h>
// #include <zephyr/drivers/led_strip.h>
// #include <string.h>

// #include <ui/ui.h>

// LOG_MODULE_REGISTER(ui);

// #define SW0_NODE DT_ALIAS(sw0)
// #if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
// #error "Unsupported board: sw0 devicetree alias is not defined"
// #endif

// #define STRIP_NODE DT_ALIAS(led_strip)
// #if DT_NODE_HAS_PROP(STRIP_NODE, chain_length)
// #define UI_STRIP_NUM_PIXELS DT_PROP(STRIP_NODE, chain_length)
// #else
// #error Unable to determine length of LED strip
// #endif

// #define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

// #define UI_LED_BRIGHTNESS 50

// static const struct led_rgb ui_color_map [SIZE_GUARD] = {
//     [WHITE]      = RGB(UI_LED_BRIGHTNESS, UI_LED_BRIGHTNESS, UI_LED_BRIGHTNESS),
//     [RED]      = RGB(UI_LED_BRIGHTNESS, 0,   0),
//     [GREEN]    = RGB(0,   UI_LED_BRIGHTNESS, 0),
//     [BLUE]     = RGB(0,   0,   UI_LED_BRIGHTNESS),
// };

// typedef struct ui_gpio_config_s {
//     const struct gpio_dt_spec button;
//     struct gpio_callback buttonCbData;
    
//     const struct device *strip;
    
//     struct k_work led_work;
    
//     bool led_is_on;
//     ui_colors_t active_color;

//     struct led_rgb pixels[UI_STRIP_NUM_PIXELS];

// } ui_gpio_config_t;

// static void ui_led_work_handler(struct k_work *work);

// static ui_gpio_config_t ui_config = {
//     .button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0}),
//     .strip = DEVICE_DT_GET(STRIP_NODE),
//     .active_color = WHITE,
//     .led_is_on = false,
//     .pixels = ui_color_map[YELLOW],
// };

// static void ui_led_work_handler(struct k_work *work)
// {
//     ui_config.led_is_on = !ui_config.led_is_on;

//     if (ui_config.led_is_on) {
//         if (ui_config.active_color < SIZE_GUARD)
//         {
//             ui_config.active_color++;
//         }
//         else
//         {
//             ui_config.active_color = 0;
//         }

//         for (size_t i = 0; i < UI_STRIP_NUM_PIXELS; i++) {
//             memcpy(&ui_config.pixels[i], &ui_color_map[ui_config.active_color], sizeof(struct led_rgb));
//         }
//         LOG_INF("LED Strip turned ON");
//     } else {
//         memset(ui_config.pixels, 0x00, sizeof(ui_config.pixels));
//         LOG_INF("LED Strip turned OFF");
//     }

//     int rc = led_strip_update_rgb(ui_config.strip, ui_config.pixels, UI_STRIP_NUM_PIXELS);
//     if (rc) {
//         LOG_ERR("Couldn't update strip: %d", rc);
//     }
// }

// static void ui_buttonCallback(const struct device *dev, struct gpio_callback *cb,
// 		    uint32_t pins)
// {
//     k_work_submit(&ui_config.led_work);
// }

// static int ui_initButton(void)
// {    
//     int errorCode = 0;

//     do
//     {
//         if (!gpio_is_ready_dt(&ui_config.button))
//         {
//             LOG_ERR("Button device is not ready");
//             errorCode = -1;
//             break;
//         }

//         errorCode = gpio_pin_configure_dt(&ui_config.button, GPIO_INPUT);
//         if (errorCode != 0) {
//             LOG_ERR("Cannot configure button device");
//             break;
//         }

//         errorCode = gpio_pin_interrupt_configure_dt(&ui_config.button,
// 					      GPIO_INT_EDGE_TO_ACTIVE);
//         if (errorCode != 0)
//         {
//             LOG_ERR("Cannot configure interrupt on button device");
//             break;
//         }

//         gpio_init_callback(&ui_config.buttonCbData, ui_buttonCallback, BIT(ui_config.button.pin));
//         gpio_add_callback(ui_config.button.port, &ui_config.buttonCbData);

//     } while(0);

//     return errorCode;
// }

// static int ui_initLed(void)
// {
//     int errorCode = 0;

//     do 
//     {
//         if (!device_is_ready(ui_config.strip)) {
//             LOG_ERR("LED strip device %s is not ready", ui_config.strip->name);
//             errorCode = -1;
//             break;
//         }

//         k_work_init(&ui_config.led_work, ui_led_work_handler);

//         memset(ui_config.pixels, 0x00, sizeof(ui_config.pixels));
//         errorCode = led_strip_update_rgb(ui_config.strip, ui_config.pixels, UI_STRIP_NUM_PIXELS);
//         if (errorCode != 0) {
//             LOG_ERR("Failed to clear strip on init");
//         }

//     } while(0);

//     return errorCode;
// }

// int ui_init(void) {
//     int errorCode = 0;
    
//     do 
//     {
//         errorCode = ui_initLed();
//         if (errorCode != 0)
//         {
//             LOG_WRN("Skipping UI LED initialization");
//         }
//         else
//         {
//             LOG_INF("Initialized UI LED Strip");
//         }
//         errorCode = ui_initButton();

//         if (errorCode != 0)
//         {
//             LOG_WRN("Skipping UI button initialization");
//         }
//         else
//         {
//             LOG_INF("Initialized UI button");
//         }

//     } while (0);

//     LOG_INF("Initialized UI");
//     return errorCode;
// }

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/led_strip.h>
#include <string.h>

#include <ui/ui.h>
// Include the header for the BLE logic we wrote previously
#include "../ble/ble_rtls.h" 

LOG_MODULE_REGISTER(ui);

/* --- Hardware Definitions --- */

#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

#define STRIP_NODE DT_ALIAS(led_strip)
#if DT_NODE_HAS_PROP(STRIP_NODE, chain_length)
#define UI_STRIP_NUM_PIXELS DT_PROP(STRIP_NODE, chain_length)
#else
#error Unable to determine length of LED strip
#endif

/* --- Configuration --- */

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }
#define UI_LED_BRIGHTNESS 20 // Low brightness to save eyes/power

// Custom Queue Stack Size
#define UI_WORK_Q_STACK_SIZE 2048
#define UI_WORK_Q_PRIORITY   5

// Define the 3 States
typedef enum {
    UI_STATE_IDLE = 0,   // White, BLE Off
    UI_STATE_BEACON,     // Green, BLE Advertising
    UI_STATE_TAG,        // Blue,  BLE Scanning
    UI_STATE_COUNT       // Total number of states
} ui_state_t;

// Map States to Colors
static const struct led_rgb state_colors[UI_STATE_COUNT] = {
    [UI_STATE_IDLE]   = RGB(UI_LED_BRIGHTNESS, UI_LED_BRIGHTNESS, UI_LED_BRIGHTNESS), // White
    [UI_STATE_BEACON] = RGB(0,   UI_LED_BRIGHTNESS, 0),                               // Green
    [UI_STATE_TAG]    = RGB(0,   0,   UI_LED_BRIGHTNESS),                               // Blue
};

typedef struct ui_gpio_config_s {
    const struct gpio_dt_spec button;
    struct gpio_callback buttonCbData;
    const struct device *strip;
    struct k_work led_work;
    
    // Track the current state of the application
    ui_state_t current_state;

    struct led_rgb pixels[UI_STRIP_NUM_PIXELS];

} ui_gpio_config_t;

// Forward declaration
static void ui_led_work_handler(struct k_work *work);

// Global Config
static ui_gpio_config_t ui_config = {
    .button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0}),
    .strip = DEVICE_DT_GET(STRIP_NODE),
    .current_state = UI_STATE_IDLE, // Start at White/Idle
};

// Define Custom Work Queue
K_THREAD_STACK_DEFINE(ui_work_q_stack, UI_WORK_Q_STACK_SIZE);
static struct k_work_q ui_work_q;

/* --- Logic --- */

static void ui_led_work_handler(struct k_work *work)
{
    int rc;

    // 1. Handle BLE State Switching
    // We do this in the work thread, not the ISR, because it might block slightly
    switch (ui_config.current_state) {
        case UI_STATE_IDLE:
            ble_rtls_setMode(RTLS_MODE_IDLE);
            LOG_INF("State: IDLE (White)");
            break;
        case UI_STATE_BEACON:
            ble_rtls_setMode(RTLS_MODE_BEACON);
            LOG_INF("State: BEACON (Green)");
            break;
        case UI_STATE_TAG:
            ble_rtls_setMode(RTLS_MODE_TAG);
            LOG_INF("State: TAG (Blue)");
            break;
        default:
            break;
    }

    // 2. Handle LED Updating
    // Get the color for the current state
    struct led_rgb color = state_colors[ui_config.current_state];

    // Fill buffer
    for (size_t i = 0; i < UI_STRIP_NUM_PIXELS; i++) {
        memcpy(&ui_config.pixels[i], &color, sizeof(struct led_rgb));
    }

    // Flush to hardware
    rc = led_strip_update_rgb(ui_config.strip, ui_config.pixels, UI_STRIP_NUM_PIXELS);
    if (rc) {
        LOG_ERR("Couldn't update strip: %d", rc);
    }
}

static void ui_buttonCallback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
    // Advance state (0 -> 1 -> 2 -> 0)
    ui_config.current_state++;
    if (ui_config.current_state >= UI_STATE_COUNT) {
        ui_config.current_state = UI_STATE_IDLE;
    }

    // Submit to our custom queue
    k_work_submit_to_queue(&ui_work_q, &ui_config.led_work);
}

static int ui_initButton(void)
{    
    int errorCode = 0;

    do
    {
        if (!gpio_is_ready_dt(&ui_config.button)) {
            LOG_ERR("Button device is not ready");
            errorCode = -1; break;
        }

        errorCode = gpio_pin_configure_dt(&ui_config.button, GPIO_INPUT);
        if (errorCode != 0) {
            LOG_ERR("Cannot configure button device");
            break;
        }

        errorCode = gpio_pin_interrupt_configure_dt(&ui_config.button, GPIO_INT_EDGE_TO_ACTIVE);
        if (errorCode != 0) {
            LOG_ERR("Cannot configure interrupt on button");
            break;
        }

        gpio_init_callback(&ui_config.buttonCbData, ui_buttonCallback, BIT(ui_config.button.pin));
        gpio_add_callback(ui_config.button.port, &ui_config.buttonCbData);

    } while(0);

    return errorCode;
}

static int ui_initLed(void)
{
    if (!device_is_ready(ui_config.strip)) {
        LOG_ERR("LED strip device %s is not ready", ui_config.strip->name);
        return -1;
    }
    
    k_work_init(&ui_config.led_work, ui_led_work_handler);
    return 0;
}

int ui_init(void) {
    int errorCode = 0;
    
    // 1. Initialize BLE subsystem first
    errorCode = ble_rtls_init();
    if (errorCode != 0) {
        LOG_ERR("Failed to init BLE");
        return errorCode;
    }

    // 2. Start UI Work Queue
    k_work_queue_start(&ui_work_q, ui_work_q_stack,
                       K_THREAD_STACK_SIZEOF(ui_work_q_stack),
                       UI_WORK_Q_PRIORITY, NULL);
    k_thread_name_set(&ui_work_q.thread, "UI_WorkQ");

    // 3. Init Hardware
    errorCode = ui_initLed();
    if (errorCode == 0) {
        ui_initButton();
        
        // 4. Set Initial State (Force update to White/Idle)
        ui_config.current_state = UI_STATE_IDLE;
        k_work_submit_to_queue(&ui_work_q, &ui_config.led_work);
        
        LOG_INF("UI Initialized");
    }

    return errorCode;
}