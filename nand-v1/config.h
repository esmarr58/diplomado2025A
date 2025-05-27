#ifndef CONFIG_H
#define CONFIG_H

// Pines de control ONFI
#define PIN_RB    15   // !RB - Ready/Busy (entrada)
#define PIN_RE    46   // !RE - Read Enable (salida)
#define PIN_CE    10   // !CE - Chip Enable (salida)
#define PIN_CLE   11   // CLE - Command Latch Enable (salida)
#define PIN_ALE   12   // ALE - Address Latch Enable (salida)
#define PIN_WE    13   // !WE - Write Enable (salida)
#define PIN_WP    14   // !WP - Write Protect (salida)

// Pines de datos IO0–IO7
#define PIN_IO0   21
#define PIN_IO1   47
#define PIN_IO2   48
#define PIN_IO3   45
#define PIN_IO4   38
#define PIN_IO5   37
#define PIN_IO6   36
#define PIN_IO7   35

// Macro para cantidad de pines de datos
#define NAND_DATA_WIDTH 8

// Pines en orden de IO0 a IO7 (para loops)
static const int nand_data_pins[NAND_DATA_WIDTH] = {
    PIN_IO0, PIN_IO1, PIN_IO2, PIN_IO3,
    PIN_IO4, PIN_IO5, PIN_IO6, PIN_IO7
};

// Delays mínimos en nanosegundos (ONFI v1.x)
#define T_CLE_ALE_NS      20   // CLE/ALE setup time
#define T_WP_NS           100  // Write pulse width
#define T_RB_TIMEOUT_MS   1    // Tiempo máximo de espera por !RB

#endif // CONFIG_H
