#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "ble_server.h"
#include "hardware/i2c.h"

#define GENERIC_MESSAGE_SEND_INTERVAL_MS 1000
#define STARTUP_SLEEP_MS 1000

// GPS DEFINES
#define UART_ID uart1
#define BAUD_RATE 9600

#define UART_TX_PIN 8
#define UART_RX_PIN 9

#define BUFFER_SIZE 128

// IMU DEFINES
#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define MPU_ADDR 0x68

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

void mpu_write(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

void mpu_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buf, len, false);
}

float offset_x = 0, offset_y = 0, offset_z = 0;

void calibrate_mpu() {
    int num_samples = 20;
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    uint8_t raw[6];

    printf("Calibrating... Keep sensor level and still.\n");

    for (int i = 0; i < num_samples; i++) {
        mpu_read(0x3B, raw, 6);
        sum_x += (int16_t)((raw[0] << 8) | raw[1]);
        sum_y += (int16_t)((raw[2] << 8) | raw[3]);
        sum_z += (int16_t)((raw[4] << 8) | raw[5]);
        sleep_ms(10);
    }

    offset_x = (float)sum_x / num_samples;
    offset_y = (float)sum_y / num_samples;
    // We expect Z to be 1g (16384), so we only subtract the deviation from 16384
    offset_z = ((float)sum_z / num_samples) - 16384.0f;

    printf("Calibration complete!\n");
}

int main() {
    stdio_init_all();

    sleep_ms(STARTUP_SLEEP_MS);

    start_server();

    /*if (!add_repeating_timer_ms(GENERIC_MESSAGE_SEND_INTERVAL_MS, periodic_message_timer_callback, NULL, &timer)) {
        printf("Failed to add repeating timer for messages!\n");
    }*/

    /*uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    char buffer[BUFFER_SIZE];
    int index = 0;

    while (true) {
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);*/

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
    /*        printf("%c", c);
        }
    }*/

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    mpu_write(0x6B, 0x00); // Wake MPU (deactivate sleep)
    mpu_write(0x1A, 0x03); // Using internal Digital Low Pass Filter
    sleep_ms(100);

    calibrate_mpu();

    uint8_t raw[6];

    while (true) {
        mpu_read(0x3B, raw, 6);

        // 1. Get Raw Data
        int16_t raw_ax = (raw[0] << 8) | raw[1];
        int16_t raw_ay = (raw[2] << 8) | raw[3];
        int16_t raw_az = (raw[4] << 8) | raw[5];

        // 2. Apply Calibration Offsets
        float cal_ax = (float)raw_ax - offset_x;
        float cal_ay = (float)raw_ay - offset_y;
        float cal_az = (float)raw_az - offset_z;

        // 4. Convert to Gs
        float final_gx = cal_ax / 16384.0f;
        float final_gy = cal_ay / 16384.0f;
        float final_gz = cal_az / 16384.0f;

        printf("X: %.2f g  Y: %.2f g  Z: %.2f g\n", final_gx, final_gy, final_gz);

        sleep_ms(50);
    }

    cancel_repeating_timer(&timer);
    return 0;
}


