#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PIN_LED 12

void app_main(void) {
    // Configurar el pin como salida
    gpio_reset_pin(PIN_LED);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);

    while (1) {
        // Encender LED
        gpio_set_level(PIN_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(500)); // Esperar 500 ms

        // Apagar LED
        gpio_set_level(PIN_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(500)); // Esperar 500 ms
    }
}
