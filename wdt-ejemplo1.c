#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"  // Necesario para ESP_LOGI y ESP_LOG_INFO

void print_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI("WDT_DEMO", "Motivo del último reinicio: %s", 
             reason == ESP_RST_TASK_WDT ? "WDT de tarea" :
             reason == ESP_RST_INT_WDT ? "WDT interrupción" :
             reason == ESP_RST_WDT ? "WDT otro" :
             "Otro motivo");
}

void app_main(void)
{
    // Configuración inicial del puerto serial
    ESP_LOGI("WDT_DEMO", "Iniciando demostración del Watchdog Timer");
    print_reset_reason();

    // 1. Demostración del WDT de tarea (Task WDT)
    ESP_LOGI("WDT_DEMO", "\n=== Demostración del WDT de tarea ===");
    
    // Habilitar el WDT para la tarea principal (app_main)
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_LOGI("WDT_DEMO", "WDT de tarea habilitado para app_main");

    for (int i = 5; i > 0; i--) {
        ESP_LOGI("WDT_DEMO", "Alimentando al WDT - Reinicio en %d segundos si no se alimenta", i);
        ESP_ERROR_CHECK(esp_task_wdt_reset());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Simular un bloqueo (no alimentar el WDT)
    ESP_LOGI("WDT_DEMO", "Simulando bloqueo - NO se alimentará más al WDT");
    while (1) {
        // ¡No llamamos a esp_task_wdt_reset() aquí!
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Este código nunca se alcanzará
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
}
