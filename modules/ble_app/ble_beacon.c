#include "ble_beacon.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>

#define DEVICE_NAME "BLE BEACON"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

LOG_MODULE_REGISTER(ble_beacon, LOG_LEVEL_DBG);

static const struct bt_data ble_beacon_advertisementData[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
    BT_DATA_BYTES(BT_DATA_SVC_DATA16,
                  0xaa, 0xfe, /* Eddystone UUID */
                  0x10,       /* Eddystone-URL frame type */
                  0x00,       /* Calibrated Tx power at 0m */
                  0x00,       /* URL Scheme Prefix http://www. */
                  'z', 'e', 'p', 'h', 'y', 'r',
                  'p', 'r', 'o', 'j', 'e', 'c', 't',
                  0x08) /* .org */
};

static const struct bt_data ble_beacon_scanResponseData[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void ble_beacon_bleReadyCb(int errorCode)
{
    char addr_s[BT_ADDR_LE_STR_LEN] = {};
    bt_addr_le_t addr = {0};
    size_t count = 1;

    do
    {
        if (errorCode != 0)
        {
            LOG_ERR("BLE initialization failed error: %d", errorCode);
            break;
        }

        LOG_DBG("BLE initialization successfull");
        errorCode = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY,
                                    ble_beacon_advertisementData, ARRAY_SIZE(ble_beacon_advertisementData),
                                    ble_beacon_scanResponseData, ARRAY_SIZE(ble_beacon_scanResponseData));
        if (errorCode != 0)
        {
            LOG_ERR("BLE Start failed error: %d", errorCode);
            break;
        }

        bt_id_get(&addr, &count);
        bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

        LOG_INF("Beacon started, advertising as %s\n", addr_s);

    } while (0);
}

int ble_beacon_init(void)
{
    const int errorCode = bt_enable(ble_beacon_bleReadyCb);
    if (errorCode != 0)
    {
        LOG_ERR("BLE enable failed");
    }

    return errorCode;
};