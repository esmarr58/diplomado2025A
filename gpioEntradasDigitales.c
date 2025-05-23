#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define PIN_BOTON GPIO_NUM_0  // Puedes cambiar el pin
#define TIEMPO_ANTIREBOTE_MS 20

void app_main(void) {
    gpio_reset_pin(PIN_BOTON);
    gpio_set_direction(PIN_BOTON, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_BOTON); // Activar resistencia de pull-up

    int estado_anterior = 1; // Valor inicial (1 = no presionado por pull-up)
    int estado_actual;
    int64_t ultima_lectura = esp_timer_get_time();

    while (1) {
        int64_t ahora = esp_timer_get_time();

        // Leer el botón
        estado_actual = gpio_get_level(PIN_BOTON);

        // Detectar flanco de bajada con anti-rebote
        if (estado_anterior == 1 && estado_actual == 0 &&
            (ahora - ultima_lectura) > (TIEMPO_ANTIREBOTE_MS * 1000)) {

            ESP_LOGI("BOTON", "¡Botón presionado!");
            ultima_lectura = ahora;
        }

        estado_anterior = estado_actual;

        vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño retardo
    }
}
