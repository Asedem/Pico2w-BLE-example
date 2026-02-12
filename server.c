#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <c++/14.3.1/math.h>
#include <c++/14.3.1/stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "ble_server.h"

#define GENERIC_MESSAGE_SEND_INTERVAL_MS 1000

#define UART_ID uart1
#define BAUD_RATE 9600

#define UART_TX_PIN 8
#define UART_RX_PIN 9

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

float convert_to_decimal(char *coord, char dir) {
    float raw = atof(coord);
    int degrees = (int)(raw / 100);
    float minutes = raw - (degrees * 100);
    float decimal = degrees + (minutes / 60.0f);
    if (dir == 'S' || dir == 'W') {
        decimal *= -1.0f;
    }
    return decimal;
}

int main() {
    stdio_init_all();

    start_server();

    /*if (!add_repeating_timer_ms(GENERIC_MESSAGE_SEND_INTERVAL_MS, periodic_message_timer_callback, NULL, &timer)) {
        printf("Failed to add repeating timer for messages!\n");
    }*/

    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    while (true) {
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            printf("%c", c);
        }
    }

    cancel_repeating_timer(&timer);
    return 0;
}


