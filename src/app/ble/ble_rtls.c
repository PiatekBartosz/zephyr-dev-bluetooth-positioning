// #include "ble/ble_beacon.h"

// #include <zephyr/bluetooth/bluetooth.h>
// #include <zephyr/logging/log.h>

// #define DEVICE_NAME "BLE BEACON"
// #define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// LOG_MODULE_REGISTER(ble_beacon, LOG_LEVEL_DBG);

// static const struct bt_data ble_beacon_advertisementData[] = {
//     BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
//     BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
//     BT_DATA_BYTES(BT_DATA_SVC_DATA16,
//                   0xaa,
//                   0xfe, /* Eddystone UUID */
//                   0x10, /* Eddystone-URL frame type */
//                   0x00, /* Calibrated Tx power at 0m */
//                   0x00, /* URL Scheme Prefix http://www. */
//                   'z',
//                   'e',
//                   'p',
//                   'h',
//                   'y',
//                   'r',
//                   'p',
//                   'r',
//                   'o',
//                   'j',
//                   'e',
//                   'c',
//                   't',
//                   0x08) /* .org */
// };

// static const struct bt_data ble_beacon_scanResponseData[] = {
//     BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
// };

// static void ble_beacon_bleReadyCb(int errorCode) {
//     char addr_s[BT_ADDR_LE_STR_LEN] = {};
//     bt_addr_le_t addr = {0};
//     size_t count = 1;

//     do {
//         if (errorCode != 0) {
//             LOG_ERR("BLE initialization failed error: %d", errorCode);
//             break;
//         }

//         LOG_DBG("BLE initialization successfull");
//         errorCode = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY,
//                                     ble_beacon_advertisementData,
//                                     ARRAY_SIZE(ble_beacon_advertisementData),
//                                     ble_beacon_scanResponseData,
//                                     ARRAY_SIZE(ble_beacon_scanResponseData));
//         if (errorCode != 0) {
//             LOG_ERR("BLE Start failed error: %d", errorCode);
//             break;
//         }

//         bt_id_get(&addr, &count);
//         bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

//         LOG_INF("Beacon started, advertising as %s\n", addr_s);

//     } while (0);
// }

// int ble_beacon_init(void) {
//     const int errorCode = bt_enable(ble_beacon_bleReadyCb);
//     if (errorCode != 0) {
//         LOG_ERR("BLE enable failed");
//     }

//     return errorCode;
// };


#include "ble_rtls.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble_rtls, LOG_LEVEL_DBG);

#define DEVICE_NAME "BLE BEACON"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// The Eddystone UUID we are looking for (0xFEAA in Little Endian)
#define EDDYSTONE_UUID_VAL_1 0xaa
#define EDDYSTONE_UUID_VAL_2 0xfe

static bool is_ble_ready = false;

/* --- Beacon Data (Same as before) --- */
static const struct bt_data ble_beacon_advertisementData[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
    BT_DATA_BYTES(BT_DATA_SVC_DATA16,
                  0xaa, 0xfe, /* Eddystone UUID */
                  0x10,       /* Eddystone-URL frame type */
                  0x00,       /* Tx Power */
                  0x00,       /* Prefix */
                  'z', 'e', 'p', 'h', 'y', 'r',
                  'p', 'r', 'o', 'j', 'e', 'c', 't',
                  0x08)       /* .org */
};

static const struct bt_data ble_beacon_scanResponseData[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static struct bt_le_scan_param scan_param = {
    .type       = BT_LE_SCAN_TYPE_ACTIVE,   // Active asks for the Name (Scan Response)
    .options    = BT_LE_SCAN_OPT_NONE,      // <--- IMPORTANT: No Duplicate Filtering
    .interval   = BT_GAP_SCAN_FAST_INTERVAL,
    .window     = BT_GAP_SCAN_FAST_WINDOW,
};

/* --- FILTERING LOGIC --- */

// Helper struct to pass status out of the parse callback
struct adv_filter_data {
    bool match_found;
};

/**
 * @brief Helper function used by bt_data_parse
 * It looks at each field in the advertisement packet.
 */
static bool parse_adv_data(struct bt_data *data, void *user_data)
{
    struct adv_filter_data *filter = (struct adv_filter_data *)user_data;

    // We are looking for Service Data (16-bit UUID)
    if (data->type == BT_DATA_SVC_DATA16) {
        
        // Data must be at least 2 bytes (just the UUID)
        if (data->data_len < 2) {
            return true; // Continue parsing
        }

        uint8_t *u8 = (uint8_t *)data->data;

        // Check if the UUID matches Eddystone (0xFEAA)
        // Zephyr stores UUIDs in Little Endian (0xAA, 0xFE)
        if (u8[0] == EDDYSTONE_UUID_VAL_1 && u8[1] == EDDYSTONE_UUID_VAL_2) {
            
            // OPTIONAL: You can filter even deeper here!
            // For example, check u8[2] for frame type 0x10 (URL)
            // if (data->data_len > 2 && u8[2] == 0x10) { ... }

            filter->match_found = true;
            return false; // Stop parsing, we found it
        }
    }

    return true; // Continue parsing other fields
}

/* --- Callbacks --- */

static void ble_rtls_scanCb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			    struct net_buf_simple *ad)
{
    // 1. Initialize filter result
    struct adv_filter_data filter = { .match_found = false };

    // 2. Parse the raw advertisement data using our helper
    // This calls `parse_adv_data` for every field in the packet
    bt_data_parse(ad, parse_adv_data, &filter);

    // 3. Only act if the filter matched
    if (filter.match_found) {
        char addr_str[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

        LOG_INF("TARGET BEACON FOUND: [%s] RSSI: %d", addr_str, rssi);
        
        // Here you can add logic to notify your UI thread if needed
    }
    // Else: Ignore packet (it's some other random Bluetooth device)
}

static void ble_rtls_readyCb(int errorCode) 
{
    if (errorCode != 0) {
        LOG_ERR("BLE initialization failed error: %d", errorCode);
        return;
    }
    is_ble_ready = true;
    LOG_INF("BLE initialization successful (IDLE)");
}

/* --- Public Functions (Same as before) --- */

int ble_rtls_init(void) 
{
    int errorCode = bt_enable(ble_rtls_readyCb);
    if (errorCode != 0) {
        LOG_ERR("BLE enable failed");
    }
    return errorCode;
}

// int ble_rtls_setMode(ble_rtls_modes_t mode)
// {
//     int err = 0;

//     if (!is_ble_ready) {
//         LOG_WRN("Cannot set mode, BLE stack not initialized yet");
//         return -EAGAIN;
//     }

//     bt_le_adv_stop();
//     bt_le_scan_stop();

//     switch (mode) {
//         case RTLS_MODE_IDLE:
//             LOG_INF("BLE Mode: IDLE (Radio Off)");
//             break;

//         case RTLS_MODE_BEACON:
//             err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY,
//                                     ble_beacon_advertisementData,
//                                     ARRAY_SIZE(ble_beacon_advertisementData),
//                                     ble_beacon_scanResponseData,
//                                     ARRAY_SIZE(ble_beacon_scanResponseData));
//             if (err) {
//                 LOG_ERR("Failed to start Beacon: %d", err);
//             } else {
//                 LOG_INF("BLE Mode: BEACON (Advertising)");
//             }
//             break;

//         case RTLS_MODE_TAG:
//             // Scan Active allows getting Scan Response data (Name), Passive does not.
//             // If you only care about the UUID in the main packet, PASSIVE is faster/lower power.
//             err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, ble_rtls_scanCb);
//             if (err) {
//                 LOG_ERR("Failed to start Tag/Scanner: %d", err);
//             } else {
//                 LOG_INF("BLE Mode: TAG (Scanning for Eddystone)");
//             }
//             break;
            
//         default:
//             return -EINVAL;
//     }

//     return err;
// }

int ble_rtls_setMode(ble_rtls_modes_t mode)
{
    int err = 0;

    if (!is_ble_ready) {
        LOG_WRN("Cannot set mode, BLE stack not initialized yet");
        return -EAGAIN;
    }

    // Stop everything before switching
    bt_le_adv_stop();
    bt_le_scan_stop();

    switch (mode) {
        case RTLS_MODE_IDLE:
            LOG_INF("BLE Mode: IDLE");
            break;

        case RTLS_MODE_BEACON:
            err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY,
                                    ble_beacon_advertisementData,
                                    ARRAY_SIZE(ble_beacon_advertisementData),
                                    ble_beacon_scanResponseData,
                                    ARRAY_SIZE(ble_beacon_scanResponseData));
            if (err) {
                LOG_ERR("Failed to start Beacon: %d", err);
            } else {
                LOG_INF("BLE Mode: BEACON (Advertising)");
            }
            break;

        case RTLS_MODE_TAG:
            // Use our custom parameters that have filtering DISABLED
            err = bt_le_scan_start(&scan_param, ble_rtls_scanCb);
            
            if (err) {
                LOG_ERR("Failed to start Tag/Scanner: %d", err);
            } else {
                LOG_INF("BLE Mode: TAG (Real-time Scanning)");
            }
            break;
            
        default:
            return -EINVAL;
    }

    return err;
}
