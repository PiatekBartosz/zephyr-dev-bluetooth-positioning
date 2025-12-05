#ifndef BLE_RTLS_H_
#define BLE_RTLS_H_

typedef enum ble_rtls_modes_e {
    RTLS_MODE_IDLE = 0,
    RTLS_MODE_BEACON,
    RTLS_MODE_TAG,
    RTLS_MODE_COUNT
} ble_rtls_modes_t;

int ble_rtls_init(void);
int ble_rtls_setMode(ble_rtls_modes_t mode);

#endif /* BLE_RTLS_H_ */
