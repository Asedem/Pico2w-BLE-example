#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize and start the BLE server
void start_server(void);

// Stop and deinitialize the BLE server
void stop_server(void);

// Send a string message via BLE notification
void send_message(const char* message);

#ifdef __cplusplus
}
#endif

#endif // BLE_SERVER_H
