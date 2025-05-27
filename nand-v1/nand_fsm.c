#include "nand_fsm.h"
#include "nand_driver.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nand_ftl.h"


static const char *TAG = "FSM";

static void send_cmd(uint8_t cmd) {
    gpio_set_level(PIN_CLE, 1);
    gpio_set_level(PIN_ALE, 0);
    nand_bus_set_output();
    nand_write_data(cmd);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);
}

static void send_addr(uint8_t addr) {
    gpio_set_level(PIN_CLE, 0);
    gpio_set_level(PIN_ALE, 1);
    nand_bus_set_output();
    nand_write_data(addr);
    nand_pulse_we();
    gpio_set_level(PIN_ALE, 0);
}

static uint8_t read_byte(void) {
    nand_bus_set_input();
    nand_pulse_re();
    return nand_read_data();
}

bool nand_fsm_reset(void) {
    ESP_LOGI(TAG, "Enviando comando RESET (0xFF)");
    send_cmd(0xFF);
    return nand_wait_ready();
}

bool nand_fsm_read_id(uint8_t *buffer, uint8_t length) {
    ESP_LOGI(TAG, "Enviando comando READ ID (0x90)");
    send_cmd(0x90);
    send_addr(0x00);

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = read_byte();
        ESP_LOGI(TAG, "Byte ID[%d] = 0x%02X", i, buffer[i]);
    }
    return true;


}

bool nand_fsm_program_page(const uint8_t *src, uint16_t block, uint8_t page) {
    ESP_LOGI("FSM", "PROGRAM PAGE: bloque=%d, página=%d", block, page);

    // Comando 1: 0x80
    gpio_set_level(PIN_CLE, 1);
    nand_bus_set_output();
    nand_write_data(0x80);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Dirección (columna 0x00 0x00)
    gpio_set_level(PIN_ALE, 1);
    nand_write_data(0x00); nand_pulse_we();  // Columna baja
    nand_write_data(0x00); nand_pulse_we();  // Columna alta

    // Dirección (página: 3 bytes)
    uint32_t addr = (block * FTL_PAGES_PER_BLOCK) + page;
    nand_write_data((addr >> 0) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 8) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 16) & 0xFF); nand_pulse_we();
    gpio_set_level(PIN_ALE, 0);

    // Enviar datos
    for (int i = 0; i < FTL_PAGE_SIZE; i++) {
        nand_write_data(src[i]);
        nand_pulse_we();
    }

    // Comando 2: 0x10
    gpio_set_level(PIN_CLE, 1);
    nand_write_data(0x10);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Esperar a que !RB esté en 1
    return nand_wait_ready();
}


bool nand_fsm_read_page(uint8_t *dest, uint16_t block, uint8_t page) {
    ESP_LOGI("FSM", "READ PAGE: bloque=%d, página=%d", block, page);

    // Comando 1: 0x00
    gpio_set_level(PIN_CLE, 1);
    nand_bus_set_output();
    nand_write_data(0x00);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Dirección (columna 0x00 0x00)
    gpio_set_level(PIN_ALE, 1);
    nand_write_data(0x00); nand_pulse_we();
    nand_write_data(0x00); nand_pulse_we();

    // Dirección de página
    uint32_t addr = (block * FTL_PAGES_PER_BLOCK) + page;
    nand_write_data((addr >> 0) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 8) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 16) & 0xFF); nand_pulse_we();
    gpio_set_level(PIN_ALE, 0);

    // Comando 2: 0x30
    gpio_set_level(PIN_CLE, 1);
    nand_write_data(0x30);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Esperar a que esté lista
    if (!nand_wait_ready()) return false;

    // Leer datos
    nand_bus_set_input();
    for (int i = 0; i < FTL_PAGE_SIZE; i++) {
        nand_pulse_re();
        dest[i] = nand_read_data();
    }

    return true;
}

bool nand_fsm_block_erase(uint16_t block) {
    ESP_LOGI("FSM", "ERASE BLOCK: bloque=%d", block);

    // Comando 0x60: preparación de borrado
    gpio_set_level(PIN_CLE, 1);
    nand_bus_set_output();
    nand_write_data(0x60);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Dirección de página (página 0 del bloque)
    uint32_t addr = block * FTL_PAGES_PER_BLOCK;
    gpio_set_level(PIN_ALE, 1);
    nand_write_data((addr >> 0) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 8) & 0xFF); nand_pulse_we();
    nand_write_data((addr >> 16) & 0xFF); nand_pulse_we();
    gpio_set_level(PIN_ALE, 0);

    // Comando 0xD0: ejecutar borrado
    gpio_set_level(PIN_CLE, 1);
    nand_write_data(0xD0);
    nand_pulse_we();
    gpio_set_level(PIN_CLE, 0);

    // Esperar a que termine
    return nand_wait_ready();
}
