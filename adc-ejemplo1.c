#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define CANAL_ADC ADC_CHANNEL_9         
#define ATENUACION_ADC ADC_ATTEN_DB_12   // Atenuación de 12 dB para rango de 0 - 3.3V
#define VREF_POR_DEFECTO 1100            // Valor de referencia de voltaje (mV)

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;

// Función para inicializar y configurar la calibración del ADC
bool inicializar_calibracion() {
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ATENUACION_ADC,
        .bitwidth = ADC_BITWIDTH_12,
    };
    return adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle) == ESP_OK;
}

// Función para inicializar y configurar el ADC
void init_adc() {
    // Configuración del ADC en modo captura única
    adc_oneshot_unit_init_cfg_t config_adc = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&config_adc, &adc_handle);

    // Configuración del canal y la atenuación
    adc_oneshot_chan_cfg_t config_canal = {
        .atten = ATENUACION_ADC,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc_handle, CANAL_ADC, &config_canal);

    // Inicializar la calibración
    
    if (!inicializar_calibracion()) {
        printf("Error al inicializar la calibración\n");
    }
    
}

void app_main() {
    // Inicialización del ADC
    init_adc();

    while (1) {
        int valor_bruto;
        adc_oneshot_read(adc_handle, CANAL_ADC, &valor_bruto);  // Leer valor con captura única

        int voltaje = 3300*valor_bruto/4096;
       
        printf("Valor ADC: %d, Voltaje: %d mV\n", valor_bruto, voltaje);

        // Esperar un segundo antes de la siguiente lectura
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
