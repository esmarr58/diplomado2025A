#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_private/rtc_clk.h"  // Header correcto para funciones RTC

static const char *TAG = "RTC_DEMO";

// Variables persistentes en RTC
RTC_DATA_ATTR static int boot_count = 0;
RTC_DATA_ATTR static time_t rtc_time = 0;

void initialize_rtc() {
    // Configuración simplificada - ESP-IDF maneja automáticamente el RTC
    ESP_LOGI(TAG, "RTC configurado automáticamente por ESP-IDF");
}

/**
 * @brief Actualiza manualmente el tiempo del RTC
 * @param new_time Opcional: Nuevo tiempo Unix a establecer (NULL para mantener auto-incremento)
 * @param increment Opcional: Segundos a incrementar (0 para mantener el valor actual)
 */
void update_rtc_time(const time_t* new_time, int increment) {
    struct timeval tv_now;
    
    // Obtener tiempo actual del sistema
    gettimeofday(&tv_now, NULL);
    
    if (new_time != NULL) {
        // Caso 1: Asignación manual directa
        rtc_time = *new_time;
        ESP_LOGI(TAG, "Tiempo RTC establecido manualmente: %lld", (long long)rtc_time);
    } 
    else if (rtc_time == 0) {
        // Caso 2: Primera inicialización
        rtc_time = tv_now.tv_sec;
        ESP_LOGI(TAG, "Tiempo RTC inicializado: %lld", (long long)rtc_time);
    }
    else {
        // Caso 3: Auto-incremento (o incremento manual)
        rtc_time += (increment != 0) ? increment : 5; // Default 5 segundos si no se especifica
        ESP_LOGI(TAG, "Tiempo RTC actualizado: %lld (+%d segundos)", 
                (long long)rtc_time, (increment != 0) ? increment : 5);
    }
    
    // Actualizar el reloj del sistema si es necesario
    if (new_time != NULL) {
        struct timeval tv = { .tv_sec = rtc_time, .tv_usec = 0 };
        settimeofday(&tv, NULL);
    }
}

void print_time() {
    struct tm timeinfo;
    localtime_r(&rtc_time, &timeinfo);
    
    char buffer[50];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Fecha y hora actual (RTC): %s", buffer);
}

void app_main(void) {
    // Inicializar el RTC
    initialize_rtc();
    
    // Incrementar contador de reinicios
    boot_count++;
    ESP_LOGI(TAG, "Contador de reinicios: %d", boot_count);
    
    // Manejo del tiempo RTC
    time_t new_time = 1748024969; 
    update_rtc_time(&new_time, 0);
    print_time();
    
    // Demostración de persistencia RTC
    if (boot_count < 3) {
        ESP_LOGI(TAG, "Entrando en modo Deep Sleep por 5 segundos...");
        esp_deep_sleep(5 * 1000000);
    } else {
        ESP_LOGI(TAG, "Demostración completada. No más deep sleep.");
    }
}
