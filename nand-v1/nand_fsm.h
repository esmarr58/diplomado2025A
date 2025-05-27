#ifndef NAND_FSM_H
#define NAND_FSM_H

#include <stdint.h>
#include <stdbool.h>

bool nand_fsm_reset(void);
bool nand_fsm_read_id(uint8_t *buffer, uint8_t length);


#endif // NAND_FSM_H
