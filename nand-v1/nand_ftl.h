#ifndef NAND_FTL_H
#define NAND_FTL_H

#include <stdint.h>
#include <stdbool.h>

// Parámetros lógicos de la NAND
#define FTL_TOTAL_BLOCKS    1024
#define FTL_PAGES_PER_BLOCK 64
#define FTL_TOTAL_PAGES     (FTL_TOTAL_BLOCKS * FTL_PAGES_PER_BLOCK)
#define FTL_PAGE_SIZE       2048
#define FTL_MAX_LBAS        4096  // Cantidad lógica de bloques visibles

typedef struct {
    uint16_t block;
    uint8_t page;
    bool valid;
} ftl_entry_t;

// Funciones de la FTL
bool ftl_init(void);
bool ftl_write_lba(uint16_t lba, const uint8_t *src);
bool ftl_read_lba(uint16_t lba, uint8_t *dest);
bool ftl_get_mapping(uint16_t lba, uint16_t *block, uint8_t *page);
bool nand_fsm_block_erase(uint16_t block);

#endif // NAND_FTL_H
