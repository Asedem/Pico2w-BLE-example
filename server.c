#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/timer.h"
#include "ble_server.h"

#define GENERIC_MESSAGE_SEND_INTERVAL_MS 1000

static repeating_timer_t timer;
static uint32_t message_counter = 0;
static char message_buffer[20];

bool periodic_message_timer_callback(repeating_timer_t *rt) {
    (void)rt;

    message_counter++;
    snprintf(message_buffer, sizeof(message_buffer), "Message %lu", message_counter);
    send_message(message_buffer);
    return true;
}

int main() {
    stdio_init_all();

    start_server();

    if (!add_repeating_timer_ms(GENERIC_MESSAGE_SEND_INTERVAL_MS, periodic_message_timer_callback, NULL, &timer)) {
        printf("Failed to add repeating timer for messages!\n");
    }

    while(true) {
        async_context_poll(cyw43_arch_async_context());
        async_context_wait_for_work_until(cyw43_arch_async_context(), at_the_end_of_time);
    }

    cancel_repeating_timer(&timer);
    return 0;
}


