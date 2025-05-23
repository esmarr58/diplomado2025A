#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024

void app_main(void) {
    // Configurar UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    const char *mensaje = "Hola desde la ESP32-S3\r\n";
    uint8_t datos[BUF_SIZE];

    while (1) {
        // Enviar mensaje
        uart_write_bytes(UART_NUM, mensaje, strlen(mensaje));

        // Leer si hay datos disponibles (no bloqueante)
        int len = uart_read_bytes(UART_NUM, datos, BUF_SIZE, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            datos[len] = '\0'; // Asegurar fin de cadena
            ESP_LOGI("UART", "Recibido: %s", datos);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo
    }
}
