#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "ble_beacon.h"
#include "app.h"

LOG_MODULE_REGISTER(app);

int app_init(void)
{
    LOG_INF("Starting zephyr ble localization application!");

    int errorCode = 0;
    do
    {
        errorCode = ble_beacon_init();
        if (errorCode != 0)
        {
            LOG_ERR("Failed to initialize ble beacon error: %d", errorCode);
            break;
        }

    } while (0);

    uint32_t appCoutner = 0;
    while (1)
    {
        k_sleep(K_SECONDS(10));
        LOG_INF("Zephyr beacon application iteration= %d!\n", appCoutner++);
    }
}