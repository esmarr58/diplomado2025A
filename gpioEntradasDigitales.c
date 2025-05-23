#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define PIN_BOTON GPIO_NUM_0  // Cambiar por el pin usado
#define TIEMPO_ANTIREBOTE_US 20000  // 20 ms en microsegundos

static const char *TAG = "BOTON";
static int64_t ultima_interrupcion = 0;

static void IRAM_ATTR isr_handler(void* arg) {
    int64_t ahora = esp_timer_get_time(); // Tiempo actual en microsegundos
    if ((ahora - ultima_interrupcion) > TIEMPO_ANTIREBOTE_US) {
        ultima_interrupcion = ahora;
        ESP_EARLY_LOGI(TAG, "¡Botón presionado!");
    }
}

void app_main(void) {
    // Configurar pin como entrada con pull-up interno
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_BOTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // Interrupción por flanco de bajada
    };
    gpio_config(&io_conf);

    // Instalar servicio de interrupciones
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_BOTON, isr_handler, NULL);

    // Ciclo vacío, la lógica está en la interrupción
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Espera pasiva
    }
}
