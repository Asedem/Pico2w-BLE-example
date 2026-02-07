#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "pico/stdlib.h"
#include "foo.h"
#include "ble_server.h"

#define ATT_CHARACTERISTIC_GENERIC_STRING_VALUE_HANDLE                 0x0009
#define ATT_CHARACTERISTIC_GENERIC_STRING_CLIENT_CONFIGURATION_HANDLE  0x000a

#define HEARTBEAT_PERIOD_MS 1000
#define APP_AD_FLAGS 0x06

#define GENERIC_MESSAGE_MAX_LEN 20

static uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    0x17, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'i', 'c', 'o', ' ', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0',
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x1a, 0x18,
};
static const uint8_t adv_data_len = sizeof(adv_data);

static int le_generic_string_notification_enabled;
static hci_con_handle_t con_handle;
static char generic_message_buffer[GENERIC_MESSAGE_MAX_LEN];
static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;

extern uint8_t const profile_data[];

static void ble_start_advertising(void) {
    uint16_t adv_int_min = 800;
    uint16_t adv_int_max = 800;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    assert(adv_data_len <= 31);

    bd_addr_t local_addr;
    gap_local_bd_addr(local_addr);
    sprintf((char*)&adv_data[12], "%02X:%02X:%02X:%02X:%02X:%02X",
        local_addr[0], local_addr[1], local_addr[2],
        local_addr[3], local_addr[4], local_addr[5]);

    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);
    printf("BLE Advertising started.\n");
}


static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);
    UNUSED(channel);
    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event_type = hci_event_packet_get_type(packet);
    switch(event_type){
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
            ble_start_advertising();
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_generic_string_notification_enabled = 0;
            printf("BLE Disconnected.\n");
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            if (le_generic_string_notification_enabled) {
                att_server_notify(con_handle, ATT_CHARACTERISTIC_GENERIC_STRING_VALUE_HANDLE, (uint8_t*)generic_message_buffer, strlen(generic_message_buffer));
            }
            break;
        default:
            break;
    }
}

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {
    UNUSED(connection_handle);

    if (att_handle == ATT_CHARACTERISTIC_GENERIC_STRING_VALUE_HANDLE) {
        return att_read_callback_handle_blob((const uint8_t *)&generic_message_buffer, strlen(generic_message_buffer), offset, buffer, buffer_size);
    }
    return 0;
}

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    if (att_handle == ATT_CHARACTERISTIC_GENERIC_STRING_CLIENT_CONFIGURATION_HANDLE) {
        le_generic_string_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        con_handle = connection_handle;
        if (le_generic_string_notification_enabled) {
            att_server_request_can_send_now_event(con_handle);
        }
        printf("Generic string notifications %sabled.\n", le_generic_string_notification_enabled ? "en" : "dis");
        return 0;
    }
    return 0;
}

static void heartbeat_handler(struct btstack_timer_source *ts) {
    btstack_run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

void start_server(void) {
    if (cyw43_arch_init()) {
        printf("BLE: failed to initialise cyw43_arch\n");
        return;
    }
    printf("BLE: CYW43 Arch initialized.\n");

    l2cap_init();
    sm_init();
    printf("BLE: L2CAP and SM initialized.\n");

    att_server_init(profile_data, att_read_callback, att_write_callback);
    printf("BLE: ATT Server initialized.\n");

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    printf("BLE: HCI Event handler added.\n");

    att_server_register_packet_handler(packet_handler);
    printf("BLE: ATT Server packet handler added.\n");

    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(&heartbeat);
    printf("BLE: Heartbeat timer started.\n");

    hci_power_control(HCI_POWER_ON);
    printf("BLE: HCI Power ON.\n");
}

void stop_server(void) {
    hci_power_control(HCI_POWER_OFF);
    btstack_run_loop_remove_timer(&heartbeat);
    cyw43_arch_deinit();
    printf("BLE: Server stopped and CYW43 deinitialized.\n");
}

void send_message(const char* message) {
    snprintf(generic_message_buffer, GENERIC_MESSAGE_MAX_LEN, "%s", message);
    if (le_generic_string_notification_enabled) {
        printf("BLE: Requesting to send generic string notification: %s\n", generic_message_buffer);
        att_server_request_can_send_now_event(con_handle);
    } else {
        printf("BLE: Generic string notifications not enabled, message not sent: %s\n", generic_message_buffer);
    }
}
