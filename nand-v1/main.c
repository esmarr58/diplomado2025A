#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "config.h"
#include "nand_driver.h"
#include "nand_fsm.h"
#include "nand_ftl.h"

#define CONFIG_TINYUSB 1
#define CONFIG_TINYUSB_MSC_ENABLED 1


static const char *TAG = "NAND_TEST";

bool comparar_buffers(const uint8_t *a, const uint8_t *b, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            ESP_LOGE("VALIDACION", "Error en byte %d: escrito=0x%02X, leído=0x%02X", i, a[i], b[i]);
            return false;
        }
    }
    return true;
}


void app_main(void) {
    nand_gpio_init();
    nand_fsm_reset();
    ftl_init();
   


    uint8_t datos[FTL_PAGE_SIZE];
    uint8_t lectura[FTL_PAGE_SIZE];

    for (int i = 0; i < FTL_PAGE_SIZE; i++) datos[i] = i % 256;

    ESP_LOGI("MAIN", "Escribiendo LBA 0...");
    ftl_write_lba(0, datos);

    ESP_LOGI("MAIN", "Leyendo LBA 0...");
    ftl_read_lba(0, lectura);

    if (comparar_buffers(datos, lectura, FTL_PAGE_SIZE)) {
        ESP_LOGI("VALIDACION", "✅ Datos verificados correctamente");
    } else {
        ESP_LOGE("VALIDACION", "❌ Fallo en validación de datos");
    }
}


