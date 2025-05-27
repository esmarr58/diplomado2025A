#ifndef NAND_DRIVER_H
#define NAND_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

void nand_gpio_init(void);
void nand_bus_set_output(void);
void nand_bus_set_input(void);
void nand_write_data(uint8_t value);
uint8_t nand_read_data(void);
void nand_pulse_we(void);
void nand_pulse_re(void);
bool nand_wait_ready(void);

#endif // NAND_DRIVER_H
