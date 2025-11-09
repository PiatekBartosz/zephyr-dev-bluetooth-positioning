#include "app.h"

#include "ble/ble_beacon.h"
#include "ui/ui.h"

#include <zephyr/smf.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app);

typedef enum app_states_e { POST, CHECK_LAST, NEUTRAL, BEACON, TAG } app_states_t;
typedef struct app_config_s { struct smf_ctx ctx; } app_config_t;

static 

int app_init(void) {
    LOG_INF("Starting zephyr ble localization application!");

    int errorCode = 0;
    do {
        errorCode = ui_init();
        if (errorCode != 0) {
            LOG_ERR("Failed to initialize ble beacon error: %d", errorCode);
            break;
        }

        errorCode = ble_beacon_init();
        if (errorCode != 0) {
            LOG_ERR("Failed to initialize ble beacon error: %d", errorCode);
            break;
        }

    } while (0);

    uint32_t appCoutner = 0;
    while (1) {
        k_sleep(K_SECONDS(10));
        LOG_INF("Zephyr beacon application iteration= %d!\n", appCoutner++);
    }
}