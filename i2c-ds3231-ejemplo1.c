#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definiciones de pines y parámetros I2C
#define I2C_MASTER_SCL_IO           9          // GPIO para SCL
#define I2C_MASTER_SDA_IO           10         // GPIO para SDA
#define DS3231_ADDRESS              0x68       // Dirección I2C del DS3231
#define I2C_MASTER_FREQ_HZ          100000     // Frecuencia del dispositivo I2C en Hz

// Tags para logs
static const char *TAG = "DS3231";

// Variables para el manejo de I2C
static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_device_handle = NULL;

// ** Conversión entre BCD y Decimal **
static uint8_t bcd_to_decimal(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

static uint8_t decimal_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// ** Inicialización del controlador I2C usando i2c-master **
static esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_new configuración: %s", esp_err_to_name(ret));
        return ret;
    }

    i2c_device_config_t dev_config = {
        .device_address = DS3231_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &i2c_device_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_bus_add_device: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Bus I2C inicializado correctamente");
    return ESP_OK;
}

// ** Configuración inicial del DS3231 (establecer hora y fecha iniciales) **
static void ds3231_set_initial_time(void) {
    uint8_t initial_time[7] = {
        decimal_to_bcd(0),   // Segundos
        decimal_to_bcd(0),   // Minutos
        decimal_to_bcd(12),  // Horas: 12 (formato 24 horas)
        decimal_to_bcd(2),   // Día de la semana (Martes)
        decimal_to_bcd(1),   // Fecha: 1
        decimal_to_bcd(1),   // Mes: Enero
        decimal_to_bcd(23)   // Año: 2023
    };

    for (uint8_t i = 0; i < 7; i++) {
        uint8_t reg_data[2] = {i, initial_time[i]};
        esp_err_t ret = i2c_master_transmit(i2c_device_handle, reg_data, sizeof(reg_data), 1000);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error al configurar el registro %d: %s", i, esp_err_to_name(ret));
        }
    }
    ESP_LOGI(TAG, "Hora inicial configurada: 12:00:00, Fecha: 01/01/2023");
}

// ** Leer la hora y fecha completas del DS3231 **
static esp_err_t ds3231_read_time(uint8_t *time_data) {
    uint8_t reg = 0x00; // Registro inicial para leer (Segundos)
    return i2c_master_transmit_receive(i2c_device_handle, &reg, 1, time_data, 7, 1000);
}

// ** Función principal **
void app_main(void) {
    // Inicializar el bus I2C
    ESP_ERROR_CHECK(i2c_master_init());

    // Configurar la hora inicial si es necesario
    ds3231_set_initial_time();

    uint8_t time_data[7]; // Buffer para almacenar los datos del DS3231

    while (1) {
        esp_err_t ret = ds3231_read_time(time_data);
        if (ret == ESP_OK) {
            // Convertir los datos de BCD a Decimal
            uint8_t seconds = bcd_to_decimal(time_data[0] & 0x7F); // Ignorar bit de formato 12/24 horas
            uint8_t minutes = bcd_to_decimal(time_data[1]);
            uint8_t hours = bcd_to_decimal(time_data[2] & 0x3F);   // Ignorar bit AM/PM si está en formato 12 horas
            uint8_t day = bcd_to_decimal(time_data[3]);
            uint8_t date = bcd_to_decimal(time_data[4]);
            uint8_t month = bcd_to_decimal(time_data[5] & 0x1F);  // Ignorar bit del siglo
            uint8_t year = bcd_to_decimal(time_data[6]);

            ESP_LOGI(TAG, "Hora: %02d:%02d:%02d, Fecha: %02d/%02d/20%02d, Día: %d",
                     hours, minutes, seconds, date, month, year, day);
        } else {
            ESP_LOGE(TAG, "Error al leer la hora: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo antes de la siguiente lectura
    }
}
