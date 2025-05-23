
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "NAND_DRIVER"

// Pines conectados a la NAND
#define PIN_CE     2
#define PIN_WE     3
#define PIN_RE     4
#define PIN_CLE    5
#define PIN_ALE    6
#define PIN_RB     7
#define PIN_D0     10  // I/O[0] hasta I/O[7] en pines consecutivos
#define DATA_PINS  8

// Variables
static void delay_timing() {
    esp_rom_delay_us(1);

}

static void set_data_direction(bool output) {
    for (int i = 0; i < DATA_PINS; i++) {
        gpio_set_direction(PIN_D0 + i, output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    }
}

static void write_data(uint8_t data) {
    for (int i = 0; i < DATA_PINS; i++) {
        gpio_set_level(PIN_D0 + i, (data >> i) & 0x01);
    }
}

static uint8_t read_data() {
    uint8_t data = 0;
    for (int i = 0; i < DATA_PINS; i++) {
        data |= (gpio_get_level(PIN_D0 + i) << i);
    }
    return data;
}

static void pulse_gpio(int pin) {
    gpio_set_level(pin, 0);
    delay_timing();
    gpio_set_level(pin, 1);
    delay_timing();
}

static void nand_send_command(uint8_t cmd) {
    gpio_set_level(PIN_CLE, 1);
    gpio_set_level(PIN_ALE, 0);
    set_data_direction(true);
    write_data(cmd);
    pulse_gpio(PIN_WE);
    gpio_set_level(PIN_CLE, 0);
}

static void nand_send_address_cycle(uint8_t addr) {
    gpio_set_level(PIN_CLE, 0);
    gpio_set_level(PIN_ALE, 1);
    set_data_direction(true);
    write_data(addr);
    pulse_gpio(PIN_WE);
    gpio_set_level(PIN_ALE, 0);
}

static void nand_wait_ready() {
    while (gpio_get_level(PIN_RB) == 0) {
        vTaskDelay(1);
    }
}

static void nand_reset() {
    nand_send_command(0xFF);
    nand_wait_ready();
    ESP_LOGI(TAG, "RESET completo.");
}

static void nand_read_id(uint8_t* buffer, size_t length) {
    nand_send_command(0x90);
    nand_send_address_cycle(0x00);
    delay_timing();
    set_data_direction(false);
    gpio_set_level(PIN_RE, 0);
    delay_timing();
    for (int i = 0; i < length; i++) {
        pulse_gpio(PIN_RE);
        buffer[i] = read_data();
    }
    gpio_set_level(PIN_RE, 1);
}

void nand_gpio_init() {
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_CE) | (1ULL << PIN_WE) | (1ULL << PIN_RE) |
                        (1ULL << PIN_CLE) | (1ULL << PIN_ALE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&cfg);

    for (int i = 0; i < DATA_PINS; i++) {
        gpio_reset_pin(PIN_D0 + i);
    }
    gpio_set_direction(PIN_RB, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_RB, GPIO_PULLUP_ONLY);

    gpio_set_level(PIN_CE, 0);
    gpio_set_level(PIN_WE, 1);
    gpio_set_level(PIN_RE, 1);
    gpio_set_level(PIN_CLE, 0);
    gpio_set_level(PIN_ALE, 0);
    ESP_LOGI(TAG, "GPIO inicializados");
}

void nand_task(void* pvParameters) {
    nand_gpio_init();
    nand_reset();

    uint8_t id[5];
    nand_read_id(id, sizeof(id));

    ESP_LOGI(TAG, "ID leído: %02X %02X %02X %02X %02X", id[0], id[1], id[2], id[3], id[4]);

    vTaskDelete(NULL);
}

void app_main(void) {
    xTaskCreatePinnedToCore(nand_task, "nand_task", 4096, NULL, 5, NULL, 0);
}
