#include "nand_driver.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"


// Inicialización de todos los pines de control y datos
void nand_gpio_init(void) {
    gpio_config_t cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    // Pines de control
    int control_pins[] = { PIN_RE, PIN_WE, PIN_CE, PIN_CLE, PIN_ALE, PIN_WP };
    for (int i = 0; i < sizeof(control_pins) / sizeof(int); i++) {
        cfg.pin_bit_mask = (1ULL << control_pins[i]);
        gpio_config(&cfg);
        gpio_set_level(control_pins[i], 1); // Nivel alto por defecto
    }

    // Pines de datos como salida inicialmente
    for (int i = 0; i < NAND_DATA_WIDTH; i++) {
        cfg.pin_bit_mask = (1ULL << nand_data_pins[i]);
        gpio_config(&cfg);
        gpio_set_level(nand_data_pins[i], 0);
    }

    // Pin !RB como entrada
    gpio_set_direction(PIN_RB, GPIO_MODE_INPUT);
}

// Cambiar pines IO0–IO7 a salida
void nand_bus_set_output(void) {
    for (int i = 0; i < NAND_DATA_WIDTH; i++) {
        gpio_set_direction(nand_data_pins[i], GPIO_MODE_OUTPUT);
    }
}

// Cambiar pines IO0–IO7 a entrada
void nand_bus_set_input(void) {
    for (int i = 0; i < NAND_DATA_WIDTH; i++) {
        gpio_set_direction(nand_data_pins[i], GPIO_MODE_INPUT);
    }
}

// Escribe un byte en el bus de datos (IO0–IO7)
void nand_write_data(uint8_t value) {
    for (int i = 0; i < NAND_DATA_WIDTH; i++) {
        gpio_set_level(nand_data_pins[i], (value >> i) & 0x01);
    }
}

// Lee un byte del bus de datos
uint8_t nand_read_data(void) {
    uint8_t value = 0;
    for (int i = 0; i < NAND_DATA_WIDTH; i++) {
        value |= gpio_get_level(nand_data_pins[i]) << i;
    }
    return value;
}

// Pulso negativo a !WE (escritura)
void nand_pulse_we(void) {
    gpio_set_level(PIN_WE, 0);
    esp_rom_delay_us(1);
    gpio_set_level(PIN_WE, 1);
}

// Pulso negativo a !RE (lectura)
void nand_pulse_re(void) {
    gpio_set_level(PIN_RE, 0);
    esp_rom_delay_us(1);
    gpio_set_level(PIN_RE, 1);
}

// Espera activa a que !RB esté en 1 (listo)
bool nand_wait_ready(void) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(PIN_RB) == 0) {
        if ((esp_timer_get_time() - start) > 1000) { // timeout de 1 ms
            return false;
        }
    }
    return true;
}
