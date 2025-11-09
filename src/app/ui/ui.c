#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <ui/ui.h>

LOG_MODULE_REGISTER(ui);

#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

typedef enum ui_states_e { POST, CHECK_LAST, NEUTRAL, BEACON, TAG } ui_states_t;

typedef struct ui_gpio_config_s {
    const struct gpio_dt_spec button;
    struct gpio_callback buttonCbData;
    const struct gpio_dt_spec led;

    ui_states_t state;

} ui_gpio_config_t;

static ui_gpio_config_t ui_config = {
    .state = POST,
    .button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0}),
    .led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0}),
};

static void ui_buttonCallback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
    LOG_INF("Button pressed");
}

static int ui_initButton(void)
{    
    int errorCode = 0;

    do
    {
        if (!gpio_is_ready_dt(&ui_config.button))
        {
            LOG_ERR("Button device is not ready");
            errorCode = -1;
            break;
        }

        errorCode = gpio_pin_configure_dt(&ui_config.button, GPIO_INPUT);
        if (errorCode != 0) {
            LOG_ERR("Cannot configure button device");
            break;
        }

        errorCode =  gpio_pin_interrupt_configure_dt(&ui_config.button,
					      GPIO_INT_EDGE_TO_ACTIVE);
        if (errorCode != 0)
        {
            LOG_ERR("Cannot configure interrupt on button device");
            break;
        }

        gpio_init_callback(&ui_config.buttonCbData, ui_buttonCallback, BIT(ui_config.button.pin));
        gpio_add_callback(ui_config.button.port, &ui_config.buttonCbData);

    } while(0);

    return errorCode;
}

static int ui_initLed(void)
{
    int errorCode = 0;

    do 
    {
        if (!gpio_is_ready_dt(&ui_config.led)) {
            LOG_ERR("LED device is not ready");
            errorCode = -1;
            break;
        }
        if (ui_config.led.port) {
            errorCode = gpio_pin_configure_dt(&ui_config.led, GPIO_OUTPUT);
            if (errorCode != 0) {
                LOG_ERR("LED configuration error");
                break;
            }
        }

    } while(0);

    return errorCode;
}

int ui_init(void) {
    int errorCode = 0;
    
    do 
    {
        errorCode = ui_initLed();
        if (errorCode != 0)
        {
            LOG_WRN("Skipping UI LED initialization");
        }
        else
        {
            LOG_INF("Initialized UI LED");
        }

        errorCode = ui_initButton();
        if (errorCode != 0)
        {
            LOG_WRN("Skipping UI button initialization");
        }
        else
        {
            LOG_INF("Initialized UI button");
        }

    } while (0);

    LOG_INF("Initialized UI");
    return errorCode;
}