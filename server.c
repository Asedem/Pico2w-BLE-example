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

#define BUFFER_SIZE 128

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

    char buffer[BUFFER_SIZE];
    int index = 0;

    while (true) {
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            /*if (c == '\n') {
                buffer[index] = '\0';

                if (strstr(buffer, "$GPRMC")) {

                    char *token;
                    int field = 0;

                    char lat[16] = {0};
                    char lon[16] = {0};
                    char lat_dir = 0;
                    char lon_dir = 0;

                    token = strtok(buffer, ",");

                    while (token != NULL) {
                        field++;

                        if (field == 4) strcpy(lat, token);
                        if (field == 5) lat_dir = token[0];
                        if (field == 6) strcpy(lon, token);
                        if (field == 7) lon_dir = token[0];

                        token = strtok(NULL, ",");
                    }

                    if (lat[0] != 0 && lon[0] != 0) {
                        float latitude = convert_to_decimal(lat, lat_dir);
                        float longitude = convert_to_decimal(lon, lon_dir);

                        printf("Lat: %.6f  Lon: %.6f", latitude, longitude);
                    }
                }

                index = 0;
            } else {
                if (index < BUFFER_SIZE - 1) {
                    buffer[index++] = c;
                }
            }*/
            printf("%c", c);
        }
    }

    cancel_repeating_timer(&timer);
    return 0;
}


