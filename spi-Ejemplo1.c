#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include <string.h>

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_CLK  18
#define PIN_CS   5  // Chip Select

#define TAG "SPI"

spi_device_handle_t spi;

void spi_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 1
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
}

void escribir_registro(uint8_t direccion, uint8_t valor) {
    uint8_t datos[3] = {0x02, direccion, valor}; // Comando de escritura
    spi_transaction_t trans = {
        .length = 8 * 3,
        .tx_buffer = datos
    };
    spi_device_transmit(spi, &trans);
    ESP_LOGI(TAG, "Escribiendo 0x%02X en dirección 0x%02X", valor, direccion);
}

uint8_t leer_registro(uint8_t direccion) {
    uint8_t tx[2] = {0x03, direccion}; // Comando de lectura
    uint8_t rx[1] = {0};

    spi_transaction_t trans = {
        .length = 8 * 3,
        .tx_buffer = tx,
        .rxlength = 8 * 3,
        .rx_buffer = rx
    };

    spi_device_transmit(spi, &trans);
    ESP_LOGI(TAG, "Leído 0x%02X de dirección 0x%02X", rx[2], direccion);
    return rx[2]; // El tercer byte recibido es el dato
}

void app_main(void) {
    spi_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    escribir_registro(0x10, 0xAB);
    vTaskDelay(pdMS_TO_TICKS(10));
    uint8_t valor = leer_registro(0x10);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
